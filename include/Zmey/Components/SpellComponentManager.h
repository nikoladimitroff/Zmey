#pragma once

#include <Zmey/EntityManager.h>
#include <Zmey/Components/ComponentRegistryCommon.h>
#include <Zmey/Components/ComponentManager.h>

#include <functional>

// TODO: Make this component extrernal
namespace Zmey
{

namespace Components
{

// Add new attributes only from here and every thing will works auto magicly
#define ITERATE_SPELL_ATTRIBUTES(MACRO) \
				MACRO(float, InitialSpeed, "initial_speed", 500)\
				MACRO(float, ImpactDamage, "impact_damage", 10)\
				MACRO(float, InitialMass, "initial_mass", 1)\
				MACRO(float, CooldownTime, "cooldown_time", 1)\
				MACRO(float, LifeTime, "life_time", 2)

struct SpellComponent : public Zmey::Components::ComponentManager
{
	DECLARE_COMPONENT_MANAGER(SpellComponent);
// TODO DECLARE_EXTERNAL_COMPONENT_MANAGER(SpellComponent);
public:
	struct EntryDescriptor
	{
#define DECLARE_ATTRIBUTES(TYPE, PROPERTY, ...) TYPE PROPERTY;
		ITERATE_SPELL_ATTRIBUTES(DECLARE_ATTRIBUTES)
		Zmey::EntityId EntityId;
	};

	virtual void InitializeFromBlob(const Zmey::tmp::vector<Zmey::EntityId>&, Zmey::MemoryInputStream&) override;
	virtual void Simulate(float deltaTime) override;
	virtual void RemoveEntity(Zmey::EntityId id) override;

	using SpellExpireDelegate = std::function<void(Zmey::EntityId id)>;

	ZMEY_API void SetSpellExpireListener(SpellExpireDelegate delegate) { m_SpellExpireCallback = delegate; };

	ZMEY_API void Push(EntryDescriptor desc);

#define DECLARE_GETTERS(TYPE, PROPERTY, ...) ZMEY_API TYPE Get##PROPERTY(Zmey::EntityId id);
	ITERATE_SPELL_ATTRIBUTES(DECLARE_GETTERS)

#define DECLARE_VECTOR_ATTRIBUTES(TYPE, PROPERTY, ...) std::vector<TYPE> m_##PROPERTY;
	ITERATE_SPELL_ATTRIBUTES(DECLARE_VECTOR_ATTRIBUTES)
	std::vector<Zmey::EntityId> m_EntityToIndex;

	SpellExpireDelegate m_SpellExpireCallback;
};

}
}
#undef DECLARE_ATTRIBUTES
#undef DECLARE_GETTERS
#undef DECLARE_VECTOR_ATTRIBUTES

