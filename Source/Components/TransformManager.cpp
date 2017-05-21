#include <Zmey/Components/TransformManager.h>

#include <nlohmann/json.hpp>

#include <Zmey/MemoryStream.h>
#include <Zmey/Components/ComponentRegistry.h>

namespace Zmey
{
namespace Components
{
TransformInstance TransformManager::Lookup(EntityId id)
{
	return TransformInstance(*this, id);
}

void TransformManager::Simulate(float deltaTime)
{
	for (auto& pos : m_Positions)
	{
		pos += Vector3(0.05f, 0.05f, 0.05f) * deltaTime;
	}
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
	m_Positions.resize(entities.size());
	size_t positionBufferLength = sizeof(Vector3) * entities.size();
	stream.Read(reinterpret_cast<uint8_t*>(&m_Positions[0]), positionBufferLength);

	m_Rotations.resize(entities.size());
	size_t rotationBufferLength = sizeof(Quaternion) * entities.size();
	stream.Read(reinterpret_cast<uint8_t*>(&m_Rotations[0]), rotationBufferLength);

	m_Scales.resize(entities.size());
	size_t scaleBufferLength = sizeof(Vector3) * entities.size();
	stream.Read(reinterpret_cast<uint8_t*>(&m_Scales[0]), scaleBufferLength);

	// Fill the entity map
	for (EntityId::IndexType i = 0u; i < entities.size(); ++i)
	{
		m_EntityToIndex[entities[i]] = i;
	}
}

DEFINE_COMPONENT_MANAGER(TransformManager, Transform, &Zmey::Components::TransformComponentDefaults, &Zmey::Components::TransformComponentToBlob);

}
}