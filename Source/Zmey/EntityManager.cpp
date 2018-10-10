#pragma once
#include <Zmey/EntityManager.h>

namespace Zmey
{

EntityId EntityManager::SpawnOne()
{
	if (m_FreeIndices.size() == 0)
	{
		m_Generation.push_back(0);
		return EntityId(static_cast<uint32_t>(m_Generation.size() - 1u), 0u);
	}
	else
	{
		uint32_t freeIndex = m_FreeIndices.front();
		m_FreeIndices.pop();
		return EntityId(freeIndex, m_Generation[freeIndex]);
	}
}

tmp::vector<EntityId> EntityManager::SpawnRange(EntityId::IndexType count)
{
	tmp::vector<EntityId> entities;
	entities.reserve(count);
	for (EntityId::IndexType i = 0; i < count; i++)
	{
		entities.push_back(SpawnOne());
	}
	return entities;
}

void EntityManager::Destroy(EntityId id)
{
	if (m_Generation[id.Index] != id.Generation)
	{
		return;
	}
	++m_Generation[id.Index];
	m_FreeIndices.push(id.Index);
}
bool EntityManager::IsAlive(EntityId id)
{
	return m_Generation[id.Index] == id.Generation;
}

}