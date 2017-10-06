#pragma once
#include <Zmey/EntityManager.h>
#include <Zmey/Components/ComponentRegistryCommon.h>
#include <Zmey/Components/ComponentManager.h>
#include <Zmey/Hash.h>

namespace Zmey
{

namespace Components
{
	
class ProjectileComponent : public ComponentManager
{
	DECLARE_COMPONENT_MANAGER(ProjectileComponent);
public:

	virtual void InitializeFromBlob(const tmp::vector<EntityId>&, Zmey::MemoryInputStream&) override;
	virtual void Simulate(float deltaTime) override;
	virtual void RemoveEntity(EntityId id) override;
private:
	stl::vector<EntityId> m_Projectiles;
};


}

}