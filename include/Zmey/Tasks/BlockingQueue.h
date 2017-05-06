#pragma once
#include <mutex>

namespace Zmey
{

template<class T>
class BlockingQueue
{
private:
	stl::deque<T> m_Queue;
	std::condition_variable m_Condition;
	std::mutex m_Mutex;
public:
	void Enqueue(T&& value)
	{
		{
			std::lock_guard<std::mutex> lock(m_Mutex);
			m_Queue.push_back(std::forward<T>(value));
		}
		m_Condition.notify_all();
	}

	T Dequeue()
	{
		std::unique_lock<std::mutex> lock(m_Mutex);
		m_Condition.wait(lock, [this]() { return m_Queue.size() != 0; });
		T first(std::move(m_Queue.front()));
		m_Queue.pop_front();
		return first;
	}
};

}