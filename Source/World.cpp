#pragma once
#include <Zmey/World.h>
#include <Zmey/Memory/MemoryManagement.h>
#include <Zmey/Components/ComponentRegistry.h>

namespace Zmey
{

World::World()
{
	using namespace Zmey::Components;
	ComponentIndex i = 0u;
	for (const ComponentManagerEntry* entry = GetComponentManagerAtIndex(0); entry; entry = GetComponentManagerAtIndex(++i))
	{
		m_ComponentManagers.push_back(entry->Instantiate());
	}
}

void World::InitializeFromBuffer(const uint8_t* buffer, size_t size)
{
	const uint8_t* bufferStart = buffer;
	tmp::small_string versionString;
	while (*buffer)
	{
		versionString.push_back(*(buffer++));
	}
	assert(versionString == "1.0");
	using IndexType = Zmey::EntityId::IndexType;
	IndexType entityCount = *reinterpret_cast<const IndexType*>(buffer);
	buffer += sizeof(IndexType);

	auto tempScope = TempAllocator::GetTlsAllocator().ScopeNow();
	tmp::vector<EntityId> entities = m_EntityManager.SpawnRange(entityCount);

	for (Zmey::ComponentIndex i = 0; i < m_ComponentManagers.size(); i++)
	{
		size_t bytesRead = m_ComponentManagers[i]->InitializeFromBlob(entities, buffer);
		buffer += bytesRead;
		ASSERT_FATAL(static_cast<size_t>(buffer - bufferStart) >= size);
	}
}

}