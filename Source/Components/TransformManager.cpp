#include <Zmey/Components/TransformManager.h>

#include <nlohmann/json.hpp>

#include <Zmey/Components/ComponentRegistry.h>

namespace Zmey
{
namespace Components
{

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

}
}
REGISTER_COMPONENT_MANAGER(Transform, &Zmey::Components::TransformComponentToBlob, nullptr); // Replace with correct implementations