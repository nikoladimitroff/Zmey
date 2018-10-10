#pragma once

#include <Zmey/Memory/MemoryManagement.h>
#include <Zmey/EntityManager.h>

namespace Zmey
{
class MemoryInputStream;
class World;
namespace Components
{

class ComponentManager
{
public:
	ComponentManager(World& world)
		: m_World(world)
	{}
	virtual ~ComponentManager() {}

	virtual void InitializeFromBlob(const tmp::vector<EntityId>& entities, MemoryInputStream& blob) = 0;
	virtual void Simulate(float deltaTime) = 0;
	virtual void RemoveEntity(EntityId id) = 0;
	inline World& GetWorld()
	{
		return m_World;
	}
private:
	World& m_World;
};

}
}