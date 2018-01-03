#include <SpellComponentManager.h>
#include <Zmey/Components/ComponentRegistry.h>

void SpellComponent::InitializeFromBlob(const Zmey::tmp::vector<Zmey::EntityId>&, Zmey::MemoryInputStream&)
{

}

template<typename T>
void NotSaveErase(T& array, size_t index)
{
	array[index] = array[array.size() - 1];
	array.resize(array.size() - 1);
}

void SpellComponent::Simulate(float deltaTime)
{
	Zmey::tmp::vector<size_t> forErase;
	for(size_t i = 0; i < m_LifeTime.size(); i++)
	{
		m_LifeTime[i] -= deltaTime;
		if (m_LifeTime[i] < 0.f)
		{
			forErase.push_back(i);
		}
	}

	for (auto it = forErase.rbegin(); it != forErase.rend(); ++it)
	{
		if (m_SpellExpireCallback)
		{
			m_SpellExpireCallback(m_EntityId[*it]);
		}
		
		NotSaveErase(m_InitialSpeed, *it);
		NotSaveErase(m_ImpactDamage, *it);
		NotSaveErase(m_InitialMass, *it);
		NotSaveErase(m_CooldownTime, *it);
		NotSaveErase(m_LifeTime, *it);
		NotSaveErase(m_EntityId, *it);
	}
}

void SpellComponent::RemoveEntity(Zmey::EntityId id)
{

}

void SpellComponent::Push(SpellComponent::EntryDescriptor desc)
{
	m_InitialSpeed.push_back(desc.InitialSpeed);
	m_ImpactDamage.push_back(desc.ImpactDamage);
	m_InitialMass.push_back(desc.InitialMass);
	m_CooldownTime.push_back(desc.CooldownTime);
	m_LifeTime.push_back(desc.LifeTime);
	m_EntityId.push_back(desc.EntityId);
}


DEFINE_EXTERNAL_COMPONENT_MANAGER(SpellComponent, Spell, nullptr, nullptr);