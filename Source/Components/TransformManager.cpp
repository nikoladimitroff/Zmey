#include <Zmey/Components/TransformManager.h>

#include <nlohmann/json.hpp>

#include <Zmey/Components/ComponentRegistry.h>

namespace Zmey
{
namespace Components
{

void TransformComponentToBlob(const nlohmann::json& rawJson, IDataBlob& blob)
{
	//float position[3];
	//position[0] = rawJson["Position"][0];
	//position[1] = rawJson["Position"][1];
	//position[2] = rawJson["Position"][2];
	//blob.WriteData("Position", reinterpret_cast<uint8_t*>(position), sizeof(position));
	//
	//float rotation[4];
	//rotation[0] = rawJson["Rotation"][0];
	//rotation[1] = rawJson["Rotation"][1];
	//rotation[2] = rawJson["Rotation"][2];
	//rotation[2] = rawJson["Rotation"][3];
	//blob.WriteData("Rotation", reinterpret_cast<uint8_t*>(rotation), sizeof(rotation));
	//
	//float scale[3];
	//scale[0] = rawJson["Scale"][0];
	//scale[1] = rawJson["Scale"][1];
	//scale[2] = rawJson["Scale"][2];
	//blob.WriteData("Scale", reinterpret_cast<uint8_t*>(scale), sizeof(scale));
}

}
}
REGISTER_COMPONENT_MANAGER(Transform, nullptr, nullptr); // Replace with correct implementations