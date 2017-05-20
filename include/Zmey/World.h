#pragma once
#include <Zmey/Memory/MemoryManagement.h>
#include <Zmey/EntityManager.h>

namespace Zmey
{

class World
{
public:
	World();
	~World();
	EntityManager& GetEntityManager()
	{
		return m_EntityManager;
	}
	template<typename T>
	T& GetManager()
	{
		return *reinterpret_cast<T*>(m_ComponentManagers[T::SZmeyComponentIndex]);
	}
	void InitializeFromBuffer(const uint8_t* buffer, size_t size);
private:
	EntityManager m_EntityManager;
	std::vector<void*> m_ComponentManagers;
};

}