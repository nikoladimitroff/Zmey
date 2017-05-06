#pragma once
#include <vector>
#include <atomic>
#include <memory>

#include <Zmey/Memory/MemoryManagement.h>
#include "BlockingQueue.h"

namespace Zmey
{
namespace Modules
{
void Initialize();
}

struct Task
{
public:
	const char* GetName() const
	{
		return m_Name;
	}

	virtual ~Task() {}
	virtual void Execute() = 0;

protected:
	Task(const char* name)
		: m_Name(name)
	{}

private:
	const char* m_Name;
};

template<typename Functor>
struct SimpleTask : public Task
{
public:
	SimpleTask(const char* name, Functor functor)
		: Task(name)
		, m_Functor(functor)
	{
	}

	virtual void Execute() override
	{
		m_Functor();
	}
private:
	Functor m_Functor;
};
using TaskPtr = stl::unique_ptr<Task>;

class WorkerThread
{
public:
	WorkerThread(BlockingQueue<TaskPtr>& taskQueue)
		: m_Tasks(taskQueue)
		, m_KeepRunning(true)
		, m_Thread(&WorkerThread::Run, this)
	{
	}
	~WorkerThread()
	{
		m_Thread.join();
	}
	void Run()
	{
		TaskPtr task;
		do
		{
			task = std::move(m_Tasks.Dequeue());
			task->Execute();
		} while (m_KeepRunning);
	}
	inline void Stop()
	{
		m_KeepRunning = false;
	}
private:
	BlockingQueue<TaskPtr>& m_Tasks;
	std::atomic_bool m_KeepRunning = true;
	std::thread m_Thread;
};

template<unsigned ThreadCount>
class TaskSystem
{
private:
	// Singleton
	TaskSystem::TaskSystem()
		: m_Workers(reinterpret_cast<WorkerThread*>(m_ThreadBuffer))
	{
		SpawnThreads();
	}

public:
	TaskSystem::~TaskSystem()
	{
		for (int i = 0; i < ThreadCount; ++i)
		{
			m_Workers[i].Stop();
		}
	}

	template<typename Functor>
	inline void SpawnTask(const char* name, Functor f)
	{
		m_TaskQueue.Enqueue(stl::make_unique<SimpleTask<Functor>>(name, f));
	}
	// Make StaticAlloc a friend so that the system can be initialized
	template<typename T>
	friend T* StaticAlloc();

private:
	WorkerThread* m_Workers;
	char m_ThreadBuffer[sizeof(WorkerThread) * ThreadCount];
	BlockingQueue<TaskPtr> m_TaskQueue;
	void SpawnThreads()
	{
		for (auto i = 0u; i < ThreadCount; ++i)
		{
			new (m_Workers + i) WorkerThread(m_TaskQueue);
		}
	}
};

}