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
	Zmey::Name nullName(Zmey::Name::NullName());
	blob.WriteData("mesh", reinterpret_cast<uint8_t*>(&nullName), sizeof(nullName));
}

void MeshComponentToBlob(const nlohmann::json& rawJson, IDataBlob& blob)
{
	if (rawJson.find("glTF_node_index") != rawJson.end())
	{
		ASSERT_FATAL(rawJson["glTF_node_index"].is_number_integer());
		unsigned meshIndex = rawJson["glTF_node_index"];
		std::string meshPath("IncineratedDataCache/mesh_" + std::to_string(meshIndex) + ".mesh");
		uint16_t pathLength = static_cast<uint16_t>(meshPath.size());
		Zmey::Name name(meshPath.c_str());
		blob.WriteData("mesh", reinterpret_cast<uint8_t*>(&name), sizeof(name));
		blob.RequestResource(meshPath.c_str(), pathLength);
	}
}

void MeshComponentManager::InitializeFromBlob(const tmp::vector<EntityId>& entities, Zmey::MemoryInputStream& stream)
{
	for (EntityId::IndexType i = 0u; i < entities.size(); ++i)
	{
		Zmey::Name name;
		stream >> name;
		ASSERT(Zmey::Modules.ResourceLoader.IsResourceReady(name));
		Graphics::MeshHandle meshHandle = *Zmey::Modules.ResourceLoader.AsMeshHandle(name);
		m_EntityToIndex[entities[i]] = uint32_t(m_Meshes.size());
		m_Meshes.push_back(meshHandle);
	}
}

void MeshComponentManager::Simulate(float deltaTime)
{
}

void MeshComponentManager::RemoveEntity(EntityId id)
{
	auto findIt = m_EntityToIndex.find(id);
	if (findIt != m_EntityToIndex.end())
	{
		// TODO: remove data from entity indexes, or keep some kind of free list for this
		m_EntityToIndex.erase(findIt);
	}
}

stl::vector<std::tuple<EntityId, Graphics::MeshHandle>> MeshComponentManager::GetMeshes()
{
	stl::vector<std::tuple<EntityId, Graphics::MeshHandle>> result;
	result.reserve(m_EntityToIndex.size());
	for (auto& en : m_EntityToIndex)
	{
		result.emplace_back(en.first, m_Meshes[en.second]);
	}

	return result;
}


DEFINE_COMPONENT_MANAGER(MeshComponentManager, Mesh, &MeshComponentDefaults, &MeshComponentToBlob);

}
}