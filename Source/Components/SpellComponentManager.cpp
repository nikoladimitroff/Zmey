#include <Zmey/Components/SpellComponentManager.h>
#include <Zmey/Components/ComponentRegistry.h>
#include <Zmey/MemoryStream.h>

namespace Zmey
{

namespace Components
{

void SpellComponent::InitializeFromBlob(const Zmey::tmp::vector<Zmey::EntityId>& entities, Zmey::MemoryInputStream& stream)
{
	Zmey::EntityId::IndexType currentEntities = static_cast<Zmey::EntityId::IndexType>(m_InitialSpeed.size());

#define IMPLEMENT_INITIALIZE_BEHAVIOUR(TYPE, PROPERTY, ...) \
	{ \
		m_##PROPERTY.resize(currentEntities + entities.size()); \
		size_t positionBufferLength = sizeof(TYPE) * entities.size(); \
		stream.Read(reinterpret_cast<uint8_t*>(&m_##PROPERTY[currentEntities]), positionBufferLength); \
	}

	ITERATE_SPELL_ATTRIBUTES(IMPLEMENT_INITIALIZE_BEHAVIOUR);

	// Fill the entity map
	for (auto it : entities)
	{
		m_EntityToIndex.push_back(it);
	}
}

void SpellComponentDefaults(Zmey::Components::IDataBlob& blob)
{
#define IMPLEMENT_DEFAULT_BEHAVIOUR(TYPE, PROPERTY, NAME, DEFAULT) \
	{ \
	TYPE value = DEFAULT; \
	blob.WriteData(NAME, reinterpret_cast<uint8_t*>(&value), sizeof(value)); \
	}

	ITERATE_SPELL_ATTRIBUTES(IMPLEMENT_DEFAULT_BEHAVIOUR);
}

void SpellComponentToBlob(const nlohmann::json& rawJson, Zmey::Components::IDataBlob& blob)
{
#define IMPLEMENT_TO_BLOB_BEHAVIOUR(TYPE, PROPERTY, NAME, DEFAULT) \
	if (rawJson.find(NAME) != rawJson.end()) \
	{ \
		/*ASSERT_FATAL(rawJson[NAME].is_boolean());*/ \
		TYPE value = rawJson[NAME]; \
		blob.WriteData(NAME, reinterpret_cast<uint8_t*>(&value), sizeof(value));\
	}

	ITERATE_SPELL_ATTRIBUTES(IMPLEMENT_TO_BLOB_BEHAVIOUR);
}

namespace
{
template<typename T>
inline void NotSaveErase(T& array, size_t index)
{
	array[index] = array[array.size() - 1];
	array.resize(array.size() - 1);
}

template<typename T>
inline size_t FindElementIndex(T& array, Zmey::EntityId id)
{
	return 	std::find(array.begin(), array.end(), id) - array.begin();
}

}

#define IMPLEMENT_GETTERS(TYPE, PROPERTY, ...) TYPE SpellComponent::Get##PROPERTY(Zmey::EntityId id) \
{ \
 auto position = FindElementIndex(m_EntityToIndex, id); \
 return m_##PROPERTY[position]; \
}

ITERATE_SPELL_ATTRIBUTES(IMPLEMENT_GETTERS);

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

#define ERASE_ACTIVE_SPELL(TYPE, PROPERTY, ...) NotSaveErase(m_##PROPERTY, *it);

	for (auto it = forErase.rbegin(); it != forErase.rend(); ++it)
	{
		if (m_SpellExpireCallback)
		{
			// TODO: Fix physx
			m_SpellExpireCallback(m_EntityToIndex[*it]);
		}
		
		ITERATE_SPELL_ATTRIBUTES(ERASE_ACTIVE_SPELL);
		NotSaveErase(m_EntityToIndex, *it);
	}
#undef ERASE_ACTIVE_SPELL
}

void SpellComponent::RemoveEntity(Zmey::EntityId id)
{
	auto position = FindElementIndex(m_EntityToIndex, id);
#define ERASE_ACTIVE_SPELL(TYPE, PROPERTY, ...) NotSaveErase(m_##PROPERTY, position);
	ITERATE_SPELL_ATTRIBUTES(ERASE_ACTIVE_SPELL);
	NotSaveErase(m_EntityToIndex, position);
#undef ERASE_ACTIVE_SPELL
}

void SpellComponent::Push(SpellComponent::EntryDescriptor desc)
{
#define PUSH_SPELL(TYPE, PROPERTY, ...) m_##PROPERTY.push_back(desc.PROPERTY);

	ITERATE_SPELL_ATTRIBUTES(PUSH_SPELL);
	m_EntityToIndex.push_back(desc.EntityId);
}

// TODO DEFINE_EXTERNAL_COMPONENT_MANAGER(SpellComponent, ProjectileSpell, SpellComponentDefaults, SpellComponentToBlob);
DEFINE_COMPONENT_MANAGER(SpellComponent, ProjectileSpell, SpellComponentDefaults, SpellComponentToBlob);
}
}