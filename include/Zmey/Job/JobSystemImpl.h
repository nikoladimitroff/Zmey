#pragma once

#include <Zmey/Config.h>
#include <Zmey/Job/JobSystem.h>
#include <Zmey/Job/Queue.h>

#include <mutex>
#include <atomic>
#include <unordered_map>

namespace Zmey
{
namespace Job
{

using FiberHandle = void*;

class JobSystemImpl : public IJobSystem
{
public:
	JobSystemImpl(uint32_t numWorkerThreads, uint32_t numFibers, uint32_t fiberStackSize);
	~JobSystemImpl();

	virtual void RunJobs(const char* name, JobDecl* jobs, uint32_t numJobs, Counter* counter = nullptr) override;
	virtual void WaitForCounter(Counter* counter, uint32_t value) override;

	virtual void Quit() override
	{
		m_Quit.store(true);
	}

	virtual void Destroy() override
	{
		delete this;
	}
private:
	void WorkerThreadEntryPoint();
	static void FiberEntryPoint(void* params);

	void CleanUpOldFiber();

	struct NextFreeFiber
	{
		FiberHandle Handle;
		unsigned Index;
	};

	struct JobData
	{
		JobDecl Job;
		Counter* Counter;
		const char* Name;
	};

	struct ReadyFiber
	{
		unsigned FiberId;
		const char* JobName;
	};

	struct WaitingFiber
	{
		unsigned FiberId;
		unsigned TargetValue;
		const char* JobName;
		std::atomic<bool> CanBeMadeReady;
	};

	NextFreeFiber GetNextFreeFiber();

	std::vector<std::thread> m_WorkerThreads;
	std::vector<FiberHandle> m_Fibers;

	Queue<JobData> m_Jobs;
	Queue<unsigned> m_FreeFibers;
	Queue<ReadyFiber> m_ReadyFibers;

	std::atomic<bool> m_Quit;

	//
	std::mutex m_WaitingFibersMutex;
	std::unordered_map<Counter*, WaitingFiber> m_WaitingFibers;
	//

	static const unsigned INVALID_FIBER_ID = -1;
	struct WorkerThreadData
	{
		const char* CurrentJobName = nullptr;
		FiberHandle InitialFiber = nullptr;
		unsigned CurrentFiberId = INVALID_FIBER_ID;
		unsigned FiberToPushToFreeList = INVALID_FIBER_ID;
		std::atomic<bool>* CanBeMadeReadyFlag = nullptr;
	};

	static thread_local WorkerThreadData tlsWorkerThreadData;
};
}
}
