#pragma once

#include <Zmey/EntityManager.h>
#include <Zmey/Components/ComponentRegistryCommon.h>
#include <Zmey/Components/ComponentManager.h>

struct SpellComponent : public Zmey::Components::ComponentManager
{
	DECLARE_EXTERNAL_COMPONENT_MANAGER(SpellComponent);
public:
	struct EntryDescriptor
	{
		float InitialSpeed;
		float ImpactDamage;
		float InitialMass;
		float CooldownTime;
		float ExpireTime;
		Zmey::EntityId EntityId;
	};

	virtual void InitializeFromBlob(const Zmey::tmp::vector<Zmey::EntityId>&, Zmey::MemoryInputStream&) override;
	virtual void Simulate(float deltaTime) override;
	virtual void RemoveEntity(Zmey::EntityId id) override;
	void Push(EntryDescriptor desc);

	std::vector<float> InitialSpeed;
	std::vector<float> ImpactDamage;
	std::vector<float> InitialMass;
	std::vector<float> CooldownTime;
	std::vector<float> ExpireTime;
	std::vector<Zmey::EntityId> EntityId;
};
