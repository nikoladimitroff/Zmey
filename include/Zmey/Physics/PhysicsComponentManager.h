#pragma once
#include <Zmey/Math/Math.h>
#include <Zmey/EntityManager.h>
#include <Zmey/Components/ComponentRegistryCommon.h>
#include <Zmey/Components/ComponentManager.h>

namespace Zmey
{

namespace Physics
{
class PhysicsActor;

class PhysicsComponentManager : public Zmey::Components::ComponentManager
{
	DECLARE_COMPONENT_MANAGER(PhysicsComponentManager);
public:
	ZMEY_API Zmey::Physics::PhysicsActor* Lookup(EntityId entity);
	virtual void InitializeFromBlob(const tmp::vector<EntityId>&, Zmey::MemoryInputStream&) override;
	virtual void Simulate(float deltaTime) override;
	virtual void RemoveEntity(EntityId id) override;
private:
	stl::unordered_map<EntityId, EntityId::IndexType> m_EntityToActor;
	stl::vector<stl::unique_ptr<Zmey::Physics::PhysicsActor>> m_Actors;
};

}

}