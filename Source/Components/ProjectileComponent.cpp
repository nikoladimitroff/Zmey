#include <Zmey/Components/ProjectileComponent.h>

#include <algorithm>
#include <nlohmann/json.hpp>

#include <Zmey/MemoryStream.h>
#include <Zmey/Components/ComponentRegistry.h>
#include <Zmey/Components/TransformManager.h>
#include <Zmey/World.h>

namespace Zmey
{
namespace Components
{

void ProjectileComponent::InitializeFromBlob(const tmp::vector<EntityId>& entities, Zmey::MemoryInputStream& input)
{
	EntityId::IndexType currentEntities = static_cast<EntityId::IndexType>(m_Projectiles.size());
	m_Projectiles.resize(currentEntities + entities.size());
	std::memcpy(&m_Projectiles[currentEntities], entities.data(), entities.size() * sizeof(Zmey::EntityId));
}

void ProjectileComponent::RemoveEntity(EntityId id)
{
	m_Projectiles.erase(std::remove(m_Projectiles.begin(), m_Projectiles.end(), id), m_Projectiles.end());
}

void ProjectileComponent::Simulate(float deltaTime)
{
	auto& transformManager = GetWorld().GetManager<Zmey::Components::TransformManager>();
	for (const auto& entity : m_Projectiles)
	{
		auto transform = transformManager.Lookup(entity);
		auto actorForwardVector = transform.Rotation() * Zmey::Vector3(0.f, 0.f, 1.f);
		transform.Position() += actorForwardVector * 5.f * deltaTime;
	}
}

DEFINE_COMPONENT_MANAGER(ProjectileComponent, Projectile, &Zmey::Components::EmptyDefaultsToBlobImplementation, &Zmey::Components::EmptyToBlobImplementation);

}
}