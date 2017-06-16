#include <Zmey/Components/MeshComponentManager.h>

#include <nlohmann/json.hpp>

#include <Zmey/Components/ComponentRegistry.h>
#include <Zmey/Components/TransformManager.h>
#include <Zmey/MemoryStream.h>
#include <Zmey/Modules.h>
#include <Zmey/World.h>

namespace Zmey
{
namespace Components
{

void MeshComponentDefaults(IDataBlob& blob)
{
	char buffer[] = { 0, 0 };
	blob.WriteData("mesh", reinterpret_cast<uint8_t*>(buffer), sizeof(buffer));
}

void MeshComponentToBlob(const nlohmann::json& rawJson, IDataBlob& blob)
{
	if (rawJson.find("mesh") != rawJson.end())
	{
		ASSERT_FATAL(rawJson["mesh"].is_string());
		std::string meshPath = rawJson["mesh"];
		uint16_t pathLength = static_cast<uint16_t>(meshPath.size());
		blob.WriteData("mesh", meshPath.c_str(), pathLength);
		blob.RequestResource(meshPath.c_str(), pathLength);
	}
}

void MeshComponentManager::InitializeFromBlob(const tmp::vector<EntityId>& entities, Zmey::MemoryInputStream& stream)
{
	for (EntityId::IndexType i = 0u; i < entities.size(); ++i)
	{
		tmp::string meshPath;
		stream >> meshPath;
		Zmey::Name name(meshPath.c_str());
		ASSERT(Zmey::Modules::ResourceLoader->IsResourceReady(name));
		Graphics::MeshHandle meshHandle = *Zmey::Modules::ResourceLoader->AsMeshHandle(name);
		m_Meshes.push_back(meshHandle);
		m_EntityToIndex[entities[i]] = i;

		GetWorld().Meshes.insert(std::make_pair(entities[i], meshHandle));
	}
}
void MeshComponentManager::Simulate(float deltaTime)
{
}
DEFINE_COMPONENT_MANAGER(MeshComponentManager, Mesh, &MeshComponentDefaults, &MeshComponentToBlob);

}
}