#pragma once
#include <Zmey/World.h>
#include <Zmey/Memory/MemoryManagement.h>
#include <Zmey/MemoryStream.h>
#include <Zmey/Components/ComponentRegistry.h>

namespace Zmey
{

World::World()
	: m_First(0)
{
	using namespace Zmey::Components;
	ComponentIndex i = 0u;
	for (const ComponentManagerEntry* entry = GetComponentManagerAtIndex(0); entry; entry = GetComponentManagerAtIndex(++i))
	{
		m_ComponentManagers.push_back(entry->Instantiate(*this));
	}
}

void World::InitializeFromBuffer(const uint8_t* buffer, size_t size)
{
	Zmey::MemoryInputStream stream(buffer, size);
	tmp::small_string versionString;
	stream >> versionString;
	assert(versionString == "1.0");

	using EntityIndex = Zmey::EntityId::IndexType;
	EntityIndex entityCount;
	stream >> entityCount;

	auto tempScope = TempAllocator::GetTlsAllocator().ScopeNow();
	tmp::vector<EntityId> entities = m_EntityManager.SpawnRange(entityCount);
	m_First = entities[0];

	while (!stream.IsEOF())
	{
		// Read the next component
		uint64_t componentHash;
		stream >> componentHash;
		auto managerEntry = Zmey::Components::GetComponentManager(componentHash);
		// Read the indices of the entities that have this component
		EntityIndex entitiesWithComponentCount;
		stream >> entitiesWithComponentCount;
		tmp::vector<EntityIndex> indicesOfEntitiesWithComponent;
		indicesOfEntitiesWithComponent.resize(entitiesWithComponentCount);
		stream.Read(reinterpret_cast<uint8_t*>(&indicesOfEntitiesWithComponent[0]), sizeof(EntityIndex) * entitiesWithComponentCount);

		// Transform the indices of the entities to their EntityIds
		// Would love to actually make a std::transform but C++ makes the resulting operation much harder to read
		tmp::vector<EntityId> entitiesWithComponent;
		entitiesWithComponent.reserve(entitiesWithComponentCount);
		for (auto index : indicesOfEntitiesWithComponent)
		{
			entitiesWithComponent.push_back(entities[index]);
		}
		// Tell the component to read its data
		m_ComponentManagers[managerEntry.Index]->InitializeFromBlob(entitiesWithComponent, stream);
	}
}

void World::Simulate(float deltaTime)
{
	for (ComponentIndex i = 0u; i < m_ComponentManagers.size(); ++i)
	{
		m_ComponentManagers[i]->Simulate(deltaTime);
	}
}


void World::AddClassToRegistry(Zmey::Hash className, const uint8_t* buffer, size_t size)
{
	stl::vector<uint8_t> bufferContainer(size);
	std::memcpy(&bufferContainer[0], buffer, size);
	m_ClassRegistry[className] = std::move(bufferContainer);
}

EntityId World::SpawnEntity(Zmey::Hash actorType)
{
	auto it = m_ClassRegistry.find(actorType);
	ASSERT_FATAL(it != m_ClassRegistry.end());
	auto entityId = m_EntityManager.SpawnOne();
	auto tempScope = TempAllocator::GetTlsAllocator().ScopeNow();
	tmp::vector<EntityId> entityVec = { entityId };

	Zmey::MemoryInputStream stream(it->second.data(), it->second.size());
	
	tmp::small_string versionString;
	stream >> versionString;
	assert(versionString == "1.0");

	uint64_t classNameHash;
	stream >> classNameHash;

	while (!stream.IsEOF())
	{
		// Read the next component
		uint64_t componentHash;
		stream >> componentHash;
		auto managerEntry = Zmey::Components::GetComponentManager(componentHash);
		// Tell the component to read its data
		m_ComponentManagers[managerEntry.Index]->InitializeFromBlob(entityVec, stream);
	}
	return entityId;
}

}