#include "GLTFLoader.h"
#include <nlohmann/json.hpp>
#include <Zmey/Graphics/Managers/MeshManager.h>

#include <fstream>

namespace Zmey
{
namespace Incinerator
{
namespace GLTFLoader
{
namespace
{
nlohmann::json FindNode(nlohmann::json gltf, unsigned nodeIndex)
{
	auto nodes = gltf["nodes"];
	assert(nodes.is_array() && nodeIndex < nodes.size());
	return nodes[nodeIndex];
}

nlohmann::json FindMesh(nlohmann::json gltf, unsigned meshIndex)
{
	auto meshes = gltf["meshes"];
	assert(meshes.is_array() && meshIndex < meshes.size());
	const auto& result = meshes[meshIndex];
	assert(result["primitives"].is_array() && result["primitives"].size() == 1);
	return result;
}

nlohmann::json FindAccessor(nlohmann::json gltf, unsigned accIndex)
{
	auto accs = gltf["accessors"];
	assert(accs.is_array() && accIndex < accs.size());
	return accs[accIndex];
}
}

bool ParseAndIncinerate(const uint8_t* gltfData,
	uint32_t gltfSize,
	const std::string& destinationFolder,
	const std::string& contentFolder,
	const std::vector<std::string>& meshFiles)
{
	auto gltf = nlohmann::json::parse(gltfData, gltfData + gltfSize);

	// Read Buffers
	assert(gltf["buffers"].is_array());
	std::vector<std::vector<uint8_t>> buffersData;
	for (auto& buf : gltf["buffers"])
	{
		assert(buf["byteLength"].is_number_unsigned());
		buffersData.emplace_back(unsigned(buf["byteLength"]));

		assert(buf["uri"].is_string());
		std::string filename = buf["uri"];
		std::ifstream bufferFile(contentFolder + filename, std::ios::binary);
		bufferFile.read((char*)buffersData.back().data(), unsigned(buf["byteLength"]));
	}

	const auto extensionLen = strlen(".mesh");
	const auto prefixLen = strlen("mesh_");

	for (const auto& meshName : meshFiles)
	{
		auto nameWithoutExtension = meshName.substr(0, meshName.size() - extensionLen);
		unsigned meshIndex = std::stoi(nameWithoutExtension.substr(prefixLen, nameWithoutExtension.size() - prefixLen));

		// meshIndex is actually node index
		const auto& node = FindNode(gltf, meshIndex);
		assert(node["mesh"].is_number_unsigned());
		const auto& mesh = FindMesh(gltf, node["mesh"]);

		std::vector<uint32_t> indices;
		{
			const auto& indicesAccessor = FindAccessor(gltf, mesh["primitives"][0]["indices"]);
			assert(indicesAccessor["count"].is_number_unsigned());
			unsigned indicesSize = indicesAccessor["count"];
			indices.reserve(indicesSize);
			for (auto i = 0u; i < indicesSize; ++i)
			{
				//auto& face = mesh->mFaces[i];
				//assert(face.mNumIndices == 3);
				//indices.push_back(face.mIndices[0]);
				//indices.push_back(face.mIndices[1]);
				//indices.push_back(face.mIndices[2]);
			}
		}


		//std::vector<Zmey::Graphics::MeshVertex> vertices;
		//vertices.reserve(mesh->mNumVertices);

		//bool hasUVs = mesh->HasTextureCoords(0);

		//for (auto i = 0u; i < mesh->mNumVertices; ++i)
		//{
		//	auto& aiVector = mesh->mVertices[i];
		//	auto& aiNormal = mesh->mNormals[i];
		//	auto& aiUV = hasUVs ? mesh->mTextureCoords[0][i] : aiVector3D(0, 0, 0);
		//	vertices.push_back(Zmey::Graphics::MeshVertex{
		//		Zmey::Vector3{ aiVector.x, aiVector.y, aiVector.z },
		//		Zmey::Vector3{ aiNormal.x, aiNormal.y, aiNormal.z },
		//		Zmey::Vector2{ aiUV.x, aiUV.y }
		//		});
		//}



		//Zmey::Graphics::MeshDataHeader data;
		//data.VerticesCount = uint64_t(vertices.size());
		//data.IndicesCount = uint64_t(indices.size());

		//Zmey::MemoryOutputStream memstream;
		//memstream.Write(reinterpret_cast<uint8_t*>(&data), sizeof(Zmey::Graphics::MeshDataHeader));
		//memstream.Write(reinterpret_cast<uint8_t*>(vertices.data()), vertices.size() * sizeof(Zmey::Graphics::MeshVertex));
		//memstream.Write(reinterpret_cast<uint8_t*>(indices.data()), indices.size() * sizeof(uint32_t));

		//std::ofstream outputFile(destinationFolder + meshName, std::ios::binary | std::ios::out | std::ios::trunc);
		//outputFile.write(reinterpret_cast<const char*>(memstream.GetData()), memstream.GetDataSize());
	}
	return true;
}
}
}
}