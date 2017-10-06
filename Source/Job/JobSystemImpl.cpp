#include <Zmey/Job/JobSystemImpl.h>
#include <Zmey/Profile.h>

#include <vector>
#include <thread>
#include <cassert>

#ifdef ZMEY_PLATFORM_WIN
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

namespace Zmey
{
namespace Job
{
// TODO: This should be in platform stuff
void SetThreadName(const char* thread)
{
	struct THREADNAME_INFO
	{
		DWORD dwType; // Must be 0x1000.
		LPCSTR szName; // Pointer to name (in user addr space).
		DWORD dwThreadID; // Thread ID (-1=caller thread).
		DWORD dwFlags; // Reserved for future use, must be zero.
	};

	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = thread;
	info.dwThreadID = -1;
	info.dwFlags = 0;

	const DWORD MS_VC_EXCEPTION = 0x406D1388;

	__try
	{
		RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
	}
}

thread_local JobSystemImpl::WorkerThreadData JobSystemImpl::tlsWorkerThreadData;

IJobSystem* CreateJobSystem(uint32_t numWorkerThreads, uint32_t numFibers, uint32_t fiberStackSize)
{
	return new JobSystemImpl(numWorkerThreads, numFibers, fiberStackSize);
}

JobSystemImpl::~JobSystemImpl()
{
	for (auto& thread : m_WorkerThreads)
	{
		thread.join();
	}

	for (auto& fiber : m_Fibers)
	{
		::DeleteFiber(fiber);
	}
}

JobSystemImpl::JobSystemImpl(uint32_t numWorkerThreads, uint32_t numFibers, uint32_t fiberStackSize)
	: m_Quit(false)
{
	PROFILE_SET_THREAD_NAME("SchedulerThread");
	// First create all the needed fibers
	m_Fibers.reserve(numFibers);

	for (auto i = 0u; i < numFibers; ++i)
	{
		m_Fibers.push_back(::CreateFiber(fiberStackSize, FiberEntryPoint, this));
		m_FreeFibers.Enqueue(i);
	}

	m_WorkerThreads.reserve(numWorkerThreads);
	for (auto i = 0u; i < numWorkerThreads; ++i)
	{
		m_WorkerThreads.emplace_back(
			std::thread(
				&JobSystemImpl::WorkerThreadEntryPoint,
				this));
	}
}

void JobSystemImpl::WorkerThreadEntryPoint()
{
	PROFILE_SET_THREAD_NAME("WorkerThread");
	SetThreadName("WorkerThread");

	tlsWorkerThreadData.InitialFiber = ::ConvertThreadToFiber(this);

	// We are fiber now and we can schedule other fibers

	// Take a fiber and schedule it

	auto freeFiber = GetNextFreeFiber();

	tlsWorkerThreadData.CurrentFiberId = freeFiber.Index;
	::SwitchToFiber(freeFiber.Handle);

	// And we are back to clean up before the thread finishes.
	::ConvertFiberToThread();
}

JobSystemImpl::NextFreeFiber JobSystemImpl::GetNextFreeFiber()
{
	unsigned freeFiberIndex;
	// Busy loop until there is a free fiber
	while (!m_FreeFibers.Dequeue(freeFiberIndex));
	return NextFreeFiber{ m_Fibers[freeFiberIndex], freeFiberIndex };
}

void JobSystemImpl::RunJobs(const char* name, JobDecl* jobs, uint32_t numJobs, Counter* counter)
{
	if (counter)
	{
		counter->Value.store(numJobs);
	}

	for (auto i = 0u; i < numJobs; ++i)
	{
		m_Jobs.Enqueue({ jobs[i], counter, name });
	}
}

void JobSystemImpl::WaitForCounter(Counter* counter, uint32_t value)
{
	assert(counter);
	assert(tlsWorkerThreadData.CurrentJobName);
	{
		std::lock_guard<std::mutex> lock(m_WaitingFibersMutex);
		// Fast out
		if (counter->Value.load() == value)
		{
			return;
		}
		PROFILE_END_BLOCK;

		assert(counter->Value.load() > value);

		// TODO: change this mutex with something better
		// NB: if a counter is waited twice this will fail epicly.
		auto& waitingFiber = m_WaitingFibers[counter];
		waitingFiber.FiberId = tlsWorkerThreadData.CurrentFiberId;
		waitingFiber.TargetValue = value;
		waitingFiber.JobName = tlsWorkerThreadData.CurrentJobName;
		waitingFiber.CanBeMadeReady = false;

		tlsWorkerThreadData.CanBeMadeReadyFlag = &waitingFiber.CanBeMadeReady;
	}

	auto freeFiber = GetNextFreeFiber();

	tlsWorkerThreadData.CurrentFiberId = freeFiber.Index;
	::SwitchToFiber(freeFiber.Handle);

	// And we are back to clean up
	CleanUpOldFiber();
}

void JobSystemImpl::FiberEntryPoint(void* params)
{
	auto system = reinterpret_cast<JobSystemImpl*>(params);

	system->CleanUpOldFiber();

	while (!system->m_Quit.load())
	{
		// First check for waiting fibers
		if (!system->m_ReadyFibers.Empty())
		{
			ReadyFiber readyFiber;
			if (!system->m_ReadyFibers.Dequeue(readyFiber))
			{
				continue;
			}

			// Remember the current fiber which needs to be pushed into the free list
			// We cannot push it in the free list because another thread can take it
			// and corrupted the fiber stack before we manage to switch to another fiber
			tlsWorkerThreadData.FiberToPushToFreeList = tlsWorkerThreadData.CurrentFiberId;

			tlsWorkerThreadData.CurrentJobName = readyFiber.JobName;
			tlsWorkerThreadData.CurrentFiberId = readyFiber.FiberId;
			PROFILE_START_BLOCK(readyFiber.JobName);

			::SwitchToFiber(system->m_Fibers[readyFiber.FiberId]);

			// And we have returned. Clean the old fiber
			system->CleanUpOldFiber();
		}
		// Take new task
		else if (!system->m_Jobs.Empty())
		{
			JobData jobData;
			if (!system->m_Jobs.Dequeue(jobData))
			{
				continue;
			}

			tlsWorkerThreadData.CurrentJobName = jobData.Name;
			PROFILE_START_BLOCK(jobData.Name);

			jobData.Job.EntryPoint(jobData.Job.Data);

			PROFILE_END_BLOCK;
			tlsWorkerThreadData.CurrentJobName = nullptr;

			// This task is done. Decrement its counter
			if (jobData.Counter)
			{
				{
					std::lock_guard<std::mutex> lock(system->m_WaitingFibersMutex);

					jobData.Counter->Value.fetch_sub(1);
					auto findIt = system->m_WaitingFibers.find(jobData.Counter);
					if (findIt != system->m_WaitingFibers.end())
					{
						if (jobData.Counter->Value.load() <= findIt->second.TargetValue)
						{
							// Busy loop on this flag. If it is false, it means that
							// this waiting thread has not switched to another fiber
							// Adding it in the readyFibersList will expose a chance
							// to corrupt the stack of the fiber if we switch to it before
							// it has switched
							while (!findIt->second.CanBeMadeReady.load());

							system->m_ReadyFibers.Enqueue(ReadyFiber{ findIt->second.FiberId, findIt->second.JobName });
							system->m_WaitingFibers.erase(findIt);
						}
					}
				}
			}
		}
	}

	// return to Thread fiber to finish threads
	::SwitchToFiber(tlsWorkerThreadData.InitialFiber);

	// This should not be reached
	assert(false);
}

void JobSystemImpl::CleanUpOldFiber()
{
	if (tlsWorkerThreadData.FiberToPushToFreeList != INVALID_FIBER_ID)
	{
		m_FreeFibers.Enqueue(tlsWorkerThreadData.FiberToPushToFreeList);
		tlsWorkerThreadData.FiberToPushToFreeList = INVALID_FIBER_ID;
	}
	else if (tlsWorkerThreadData.CanBeMadeReadyFlag)
	{
		// Flag that we have switched the thread and it is safe to be put in
		// ready fibers list
		tlsWorkerThreadData.CanBeMadeReadyFlag->store(true);
		tlsWorkerThreadData.CanBeMadeReadyFlag = nullptr;
	}
}

}
}

#else
#error Implement me
#endif