#pragma once
#include <Zmey/Memory/MemoryManagement.h>
#include <Zmey/EntityManager.h>
#include <Zmey/Hash.h>
#include <Zmey/Components/ComponentRegistryCommon.h>
#include <Zmey/Components/ComponentManager.h>

namespace Zmey
{

class World
{
public:
	World();
	EntityManager& GetEntityManager()
	{
		return m_EntityManager;
	}
	template<typename T>
	T& GetManager()
	{
		return *reinterpret_cast<T*>(m_ComponentManagers[T::SZmeyComponentManagerIndex]);
	}
	// Get manager by its index. This is meant to be used from scripting only, use the other overload from CPP
	Components::ComponentManager& GetManager(ComponentIndex index)
	{
		return *m_ComponentManagers[index];
	}
	Zmey::EntityId GetFirstEntity() const
	{
		return m_First;
	};
	void InitializeFromBuffer(const uint8_t* buffer, size_t size);
	void AddClassToRegistry(Zmey::Hash className, const uint8_t* buffer, size_t size);
	EntityId SpawnEntity(Zmey::Hash actorClass);
	void Simulate(float deltaTime);
private:
	Zmey::EntityId m_First;
	EntityManager m_EntityManager;
	stl::vector<Components::ComponentManager*> m_ComponentManagers;
	stl::unordered_map<Zmey::Hash, stl::vector<uint8_t>> m_ClassRegistry;
};

}