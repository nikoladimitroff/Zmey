#pragma once
#include <Zmey/Math/Math.h>
#include <Zmey/EntityManager.h>
#include <Zmey/Components/ComponentRegistryCommon.h>
#include <Zmey/Components/ComponentManager.h>

namespace Zmey
{

namespace Physics
{
	
class PhysicsComponentManager : public Zmey::Components::ComponentManager
{
	DECLARE_COMPONENT_MANAGER(PhysicsComponentManager);
public:
	virtual void InitializeFromBlob(const tmp::vector<EntityId>&, Zmey::MemoryInputStream&) override;
	virtual void Simulate(float deltaTime) override;
	virtual void RemoveEntity(EntityId id) override {}
private:
};

}

}