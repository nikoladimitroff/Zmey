#include <SpellComponentManager.h>
#include <Zmey/Components/ComponentRegistry.h>

void SpellComponent::InitializeFromBlob(const Zmey::tmp::vector<Zmey::EntityId>&, Zmey::MemoryInputStream&)
{

}

void SpellComponent::Simulate(float deltaTime)
{

}

void SpellComponent::RemoveEntity(Zmey::EntityId id)
{

}

void SpellComponent::Push(SpellComponent::EntryDescriptor desc)
{
	InitialSpeed.push_back(desc.InitialSpeed);
	ImpactDamage.push_back(desc.ImpactDamage);
	InitialMass.push_back(desc.InitialMass);
	CooldownTime.push_back(desc.CooldownTime);
	ExpireTime.push_back(desc.ExpireTime);
	EntityId.push_back(desc.EntityId);
}


DEFINE_EXTERNAL_COMPONENT_MANAGER(SpellComponent, Spell, nullptr, nullptr);