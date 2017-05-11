#pragma once
#include <Zmey/EntityManager.h>

namespace Zmey
{

EntityId EntityManager::Spawn(const EntityDescription&)
{
	m_Generation.push_back(0);
	return EntityId(static_cast<uint32_t>(m_Generation.size() - 1u), 0u);
}
void EntityManager::Destroy(EntityId id)
{
	m_Generation.erase(m_Generation.begin() + id.Index);
}
bool EntityManager::IsAlive(EntityId id)
{
	return m_Generation[id.Index] == id.Generation;
}

}