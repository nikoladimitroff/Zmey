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
	auto& physEngine = *Zmey::Modules::PhysicsEngine;
	physEngine.SetWorld(GetWorld());

	Zmey::Physics::PhysicsMaterialDescription defaultMat;
	defaultMat.Friction = 0.5;
	defaultMat.Restitution = 0.5;
	defaultMat.Density = 0.5;
	physEngine.CreatePhysicsMaterial(Zmey::Name("default"), defaultMat);

	Zmey::Physics::PhysicsActorDescription actorDescription;
	actorDescription.IsStatic = true;
	actorDescription.Mass = 70;
	actorDescription.IsTrigger = false;
	actorDescription.Material = Zmey::Name("default");
	auto capsuleGeometry = physEngine.CreateSphereGeometry(0.1f);
	actorDescription.Geometry = capsuleGeometry.get();
	for (auto& entityId : entities)
	{
		auto actor = physEngine.CreatePhysicsActor(entityId, actorDescription);
		m_Actors.push_back(std::move(actor));
		m_EntityToActor[entityId] = static_cast<EntityId::IndexType>(m_Actors.size() - 1u);
	}
}

void PhysicsComponentManager::RemoveEntity(EntityId id)
{
	auto index = m_EntityToActor[id];
	m_EntityToActor.erase(id);
	// TODO: remove me from the actors list and refill m_EntityToActor map
	m_Actors[index].reset();
}

Zmey::Physics::PhysicsActor* PhysicsComponentManager::Lookup(EntityId entity)
{
	auto it = m_EntityToActor.find(entity);
	if (it != m_EntityToActor.end())
	{
		return m_Actors[it->second].get();
	}
	return nullptr;
}

DEFINE_COMPONENT_MANAGER(PhysicsComponentManager, Physics, &Zmey::Components::EmptyDefaultsToBlobImplementation, &Zmey::Components::EmptyToBlobImplementation);

}
}