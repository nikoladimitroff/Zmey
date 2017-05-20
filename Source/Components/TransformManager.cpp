#include <Zmey/Components/TransformManager.h>

#include <nlohmann/json.hpp>

#include <Zmey/Components/ComponentRegistry.h>

namespace Zmey
{
namespace Components
{
TransformManager::TransformManager()
{
	
}

TransformInstance TransformManager::Lookup(EntityId id)
{
	return TransformInstance(*this, id);
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

size_t TransformComponentFromBlob(void* managerPtr, const tmp::vector<EntityId>& entities, const uint8_t* blob)
{
	TransformManager& manager = *reinterpret_cast<TransformManager*>(managerPtr);
	manager.m_Positions.resize(entities.size());
	size_t positionBufferLength = sizeof(Vector3) * entities.size();
	std::memcpy(&manager.m_Positions[0], blob, positionBufferLength);
	blob += positionBufferLength;

	manager.m_Rotations.resize(entities.size());
	size_t rotationBufferLength = sizeof(Quaternion) * entities.size();
	std::memcpy(&manager.m_Rotations[0], blob, rotationBufferLength);
	blob += rotationBufferLength;

	manager.m_Scales.resize(entities.size());
	size_t scaleBufferLength = sizeof(Vector3) * entities.size();
	std::memcpy(&manager.m_Scales[0], blob, scaleBufferLength);
	blob += scaleBufferLength;

	return positionBufferLength + rotationBufferLength + scaleBufferLength;
}

DEFINE_COMPONENT_MANAGER(TransformManager, Transform,
	&Zmey::Components::TransformComponentToBlob,
	&Zmey::Components::TransformComponentFromBlob);

}
}