#include <Zmey/Components/TransformManager.h>

#include <nlohmann/json.hpp>

#include <Zmey/MemoryStream.h>
#include <Zmey/Components/ComponentRegistry.h>

namespace Zmey
{
namespace Components
{
TransformInstance& TransformManager::Lookup(EntityId id)
{
	return *(new TransformInstance(*this, id));
}

void TransformManager::Simulate(float deltaTime)
{
}

void TransformManager::AddNewEntity(EntityId id, Vector3 pos, Vector3 scale, Quaternion rot)
{
	m_Positions.push_back(pos);
	m_Scales.push_back(scale);
	m_Rotations.push_back(rot);
	m_EntityToIndex[id] = unsigned(m_Positions.size() - 1);
}

void TransformComponentDefaults(IDataBlob& blob)
{
	float position[3] = { 0.f, 0.f, 0.f };
	blob.WriteData("position", reinterpret_cast<uint8_t*>(position), sizeof(position));
	float rotation[4] = { 0.f, 0.f, 0.f, 1.f };
	blob.WriteData("rotation", reinterpret_cast<uint8_t*>(rotation), sizeof(rotation));
	float scale[3] = { 1.f, 1.f, 1.f };
	blob.WriteData("scale", reinterpret_cast<uint8_t*>(scale), sizeof(scale));
}

void TransformComponentToBlob(const nlohmann::json& rawJson, IDataBlob& blob)
{
	if (rawJson.find("position") != rawJson.end())
	{
		ASSERT_FATAL(rawJson["position"].is_array());
		float position[3];
		position[0] = rawJson["position"][0];
		position[1] = rawJson["position"][1];
		position[2] = rawJson["position"][2];
		blob.WriteData("position", reinterpret_cast<uint8_t*>(position), sizeof(position));
	}

	if (rawJson.find("rotation") != rawJson.end())
	{
		ASSERT_FATAL(rawJson["rotation"].is_array());
		float rotation[4];
		rotation[0] = rawJson["rotation"][0];
		rotation[1] = rawJson["rotation"][1];
		rotation[2] = rawJson["rotation"][2];
		rotation[2] = rawJson["rotation"][3];
		blob.WriteData("rotation", reinterpret_cast<uint8_t*>(rotation), sizeof(rotation));
	}

	if (rawJson.find("scale") != rawJson.end())
	{
		ASSERT_FATAL(rawJson["scale"].is_array());
		float scale[3];
		scale[0] = rawJson["scale"][0];
		scale[1] = rawJson["scale"][1];
		scale[2] = rawJson["scale"][2];
		blob.WriteData("scale", reinterpret_cast<uint8_t*>(scale), sizeof(scale));
	}
}

void TransformManager::InitializeFromBlob(const tmp::vector<EntityId>& entities, Zmey::MemoryInputStream& stream)
{
	EntityId::IndexType currentEntities = static_cast<EntityId::IndexType>(m_Positions.size());
	m_Positions.resize(currentEntities + entities.size());
	size_t positionBufferLength = sizeof(Vector3) * entities.size();
	stream.Read(reinterpret_cast<uint8_t*>(&m_Positions[currentEntities]), positionBufferLength);

	m_Rotations.resize(currentEntities + entities.size());
	size_t rotationBufferLength = sizeof(Quaternion) * entities.size();
	stream.Read(reinterpret_cast<uint8_t*>(&m_Rotations[currentEntities]), rotationBufferLength);

	m_Scales.resize(currentEntities + entities.size());
	size_t scaleBufferLength = sizeof(Vector3) * entities.size();
	stream.Read(reinterpret_cast<uint8_t*>(&m_Scales[currentEntities]), scaleBufferLength);

	// Fill the entity map
	for (EntityId::IndexType i = 0u; i < entities.size(); ++i)
	{
		m_EntityToIndex[entities[i]] = currentEntities + i;
	}
}

DEFINE_COMPONENT_MANAGER(TransformManager, Transform, &Zmey::Components::TransformComponentDefaults, &Zmey::Components::TransformComponentToBlob);

}
}