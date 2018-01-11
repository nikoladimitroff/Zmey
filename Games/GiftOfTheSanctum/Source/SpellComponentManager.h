#pragma once

#include <Zmey/EntityManager.h>
#include <Zmey/Components/ComponentRegistryCommon.h>
#include <Zmey/Components/ComponentManager.h>

#include <functional>

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
		float LifeTime;
		Zmey::EntityId EntityId;
	};

	virtual void InitializeFromBlob(const Zmey::tmp::vector<Zmey::EntityId>&, Zmey::MemoryInputStream&) override;
	virtual void Simulate(float deltaTime) override;
	virtual void RemoveEntity(Zmey::EntityId id) override;

	using SpellExpireDelegate = std::function<void(Zmey::EntityId id)>;

	void SetSpellExpireListener(SpellExpireDelegate delegate) { m_SpellExpireCallback = delegate; };

	void Push(EntryDescriptor desc);

	std::vector<float> m_InitialSpeed;
	std::vector<float> m_ImpactDamage;
	std::vector<float> m_InitialMass;
	std::vector<float> m_CooldownTime;
	std::vector<float> m_LifeTime;
	std::vector<Zmey::EntityId> m_EntityId;

	SpellExpireDelegate m_SpellExpireCallback;
};
