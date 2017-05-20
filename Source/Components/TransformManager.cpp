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

void TransformManager::Simulate(float deltaMs)
{
	for (auto& pos : m_Positions)
	{
		pos += Vector3(0.0001f, 0.0001f, 0.0001f);
	}
}

void TransformComponentToBlob(const nlohmann::json& rawJson, IDataBlob& blob)
{
	float position[3];
	position[0] = rawJson["position"][0];
	position[1] = rawJson["position"][1];
	position[2] = rawJson["position"][2];
	blob.WriteData("position", reinterpret_cast<uint8_t*>(position), sizeof(position));

	float rotation[4];
	rotation[0] = rawJson["rotation"][0];
	rotation[1] = rawJson["rotation"][1];
	rotation[2] = rawJson["rotation"][2];
	rotation[2] = rawJson["rotation"][3];
	blob.WriteData("rotation", reinterpret_cast<uint8_t*>(rotation), sizeof(rotation));

	float scale[3];
	scale[0] = rawJson["scale"][0];
	scale[1] = rawJson["scale"][1];
	scale[2] = rawJson["scale"][2];
	blob.WriteData("scale", reinterpret_cast<uint8_t*>(scale), sizeof(scale));
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
}

DEFINE_COMPONENT_MANAGER(TransformManager, Transform, &Zmey::Components::TransformComponentToBlob);

}
}