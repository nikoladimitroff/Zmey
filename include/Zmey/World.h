#pragma once
#include <Zmey/Memory/MemoryManagement.h>
#include <Zmey/EntityManager.h>

namespace Zmey
{

class World
{
public:
	EntityManager& GetEntityManager()
	{
		return m_EntityManager;
	}
private:
	EntityManager m_EntityManager;
};

}