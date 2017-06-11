#include <Zmey/Components/TagManager.h>

#include <algorithm>
#include <nlohmann/json.hpp>

#include <Zmey/MemoryStream.h>
#include <Zmey/Components/ComponentRegistry.h>
#include <Zmey/World.h>

namespace Zmey
{
namespace Components
{

void TagComponentDefaults(IDataBlob& blob)
{
	blob.WriteData("tags", reinterpret_cast<uint8_t*>(&Zmey::Name::NullName()), sizeof(Zmey::Name));
}

void TagComponentToBlob(const nlohmann::json& rawJson, IDataBlob& blob)
{
	if (rawJson.find("tags") != rawJson.end())
	{
		auto& tagArray = rawJson["tags"];
		ASSERT_FATAL(tagArray.is_array());
		tmp::small_vector<Zmey::Name> tags;
		for (auto& it : tagArray)
		{
			ASSERT_FATAL(it.is_string());
			std::string tag = it;
			Name tagHashed(tag.c_str());
			tags.push_back(tagHashed);
		}
		tags.push_back(Zmey::Name::NullName());
		blob.WriteData("tags", reinterpret_cast<uint8_t*>(tags.data()), static_cast<uint16_t>(sizeof(Zmey::Name) * tags.size()));
	}
}
void TagManager::InitializeFromBlob(const tmp::vector<EntityId>& entities, Zmey::MemoryInputStream& input)
{
	for (auto& entityId : entities)
	{
		Zmey::Name nextTag(Zmey::Name::NullName());
		for (;;)
		{
			input.Read(reinterpret_cast<uint8_t*>(&nextTag), sizeof(Zmey::Name));
			if (nextTag == Zmey::Name::NullName())
			{
				break;
			}
			m_Tags.push_back(EntityTagPair{ entityId, nextTag });
		}
	}
}


bool TagManager::HasTag(EntityId entity, Zmey::Name tag) const
{
	EntityTagPair pair{ entity, tag };
	return std::find(m_Tags.cbegin(), m_Tags.cend(), pair) != m_Tags.cend();
}
EntityId TagManager::FindFirstByTag(Zmey::Name tag) const
{
	auto it = std::find_if(m_Tags.cbegin(), m_Tags.cend(), [tag](const EntityTagPair& pair)
	{
		return pair.Tag == tag;
	});
	if (it != m_Tags.cend())
	{
		return it->Entity;
	}
	return Zmey::EntityId::NullEntity();
}

tmp::vector<EntityId> TagManager::FindAllByTag(Zmey::Name tag) const
{
	tmp::vector<EntityId> entities;
	for (auto& pair : m_Tags)
	{
		if (pair.Tag == tag)
		{
			entities.push_back(pair.Entity);
		}
	}
	return entities;
}

DEFINE_COMPONENT_MANAGER(TagManager, Tag, &Zmey::Components::TagComponentDefaults, &Zmey::Components::TagComponentToBlob);

}
}