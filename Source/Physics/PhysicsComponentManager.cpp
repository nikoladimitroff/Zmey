#include <Zmey/Physics/PhysicsComponentManager.h>

#include <nlohmann/json.hpp>

#include <Zmey/MemoryStream.h>
#include <Zmey/Components/ComponentRegistry.h>
#include <Zmey/Modules.h>

namespace Zmey
{
namespace Physics
{

void PhysicsComponentManager::Simulate(float deltaTime)
{
}

void PhysicsComponentManager::InitializeFromBlob(const tmp::vector<EntityId>& entities, Zmey::MemoryInputStream& stream)
{
	Zmey::Physics::PhysicsMaterialDescription defaultMat;
	defaultMat.Friction = 0.5;
	defaultMat.Restitution = 0.5;
	defaultMat.Density = 0.5;
	auto& physEngine = *Zmey::Modules::PhysicsEngine;
	physEngine.CreatePhysicsMaterial(Zmey::Name("default"), defaultMat);

	Zmey::Physics::PhysicsActorDescription actorDescription;
	actorDescription.IsStatic = false;
	actorDescription.Mass = 70;
	actorDescription.IsTrigger = false;
	actorDescription.Material = Zmey::Name("default");
	auto capsuleGeometry = physEngine.CreateSphereGeometry(50.f);
	actorDescription.Geometry = capsuleGeometry.get();
	for (auto& entityId : entities)
	{
		physEngine.CreatePhysicsActor(entityId, actorDescription);
	}
}

DEFINE_COMPONENT_MANAGER(PhysicsComponentManager, Physics, &Zmey::Components::EmptyDefaultsToBlobImplementation, &Zmey::Components::EmptyToBlobImplementation);

}
}