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

World::~World()
{
	using namespace Zmey::Components;
	ComponentIndex i = 0u;
	for (const ComponentManagerEntry* entry = GetComponentManagerAtIndex(0); entry; entry = GetComponentManagerAtIndex(++i))
	{
		entry->Destroy(m_ComponentManagers[i]);
	}
}

}