#pragma once

#include <Zmey/Config.h>
#include <queue>

#ifdef ZMEY_PLATFORM_WIN

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

namespace Zmey
{
namespace Job
{
// Thread-safe lockfull FIFO Queue Multi-consumer, Multi-producer
// TODO: Maybe it needs to be lockless to be high perf
template<typename T>
class Queue
{
public:
	Queue()
	{
		InitializeCriticalSectionAndSpinCount(&m_Lock, 1024);
	}

	~Queue()
	{
		DeleteCriticalSection(&m_Lock);
	}

	void Enqueue(const T& value)
	{
		::EnterCriticalSection(&m_Lock);

		m_Data.push(value);

		::LeaveCriticalSection(&m_Lock);
	}

	bool Dequeue(T& output)
	{
		::EnterCriticalSection(&m_Lock);

		if (m_Data.empty())
		{
			::LeaveCriticalSection(&m_Lock);
			return false;
		}

		output = m_Data.front();
		m_Data.pop();

		::LeaveCriticalSection(&m_Lock);
		return true;
	}

	bool Empty()
	{
		::EnterCriticalSection(&m_Lock);

		auto result = m_Data.empty();

		::LeaveCriticalSection(&m_Lock);
		return result;
	}
private:
	::CRITICAL_SECTION m_Lock;
	std::queue<T> m_Data;
};
}
}
#else
#error Implement for other platforms
#endif