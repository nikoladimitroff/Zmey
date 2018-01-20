#include "GLTFLoader.h"
#include <nlohmann/json.hpp>
#include <Zmey/Graphics/Managers/MeshManager.h>
#include <Zmey/MemoryStream.h>

#include <fstream>

namespace Zmey
{
namespace Incinerator
{
namespace GLTFLoader
{
namespace
{
nlohmann::json FindNode(const nlohmann::json& gltf, unsigned nodeIndex)
{
	auto nodes = gltf["nodes"];
	assert(nodes.is_array() && nodeIndex < nodes.size());
	return nodes[nodeIndex];
}

nlohmann::json FindMesh(const nlohmann::json& gltf, unsigned meshIndex)
{
	auto meshes = gltf["meshes"];
	assert(meshes.is_array() && meshIndex < meshes.size());
	const auto& result = meshes[meshIndex];
	assert(result["primitives"].is_array() && result["primitives"].size() == 1);
	return result;
}

nlohmann::json FindAccessor(const nlohmann::json& gltf, unsigned accIndex)
{
	auto accs = gltf["accessors"];
	assert(accs.is_array() && accIndex < accs.size());
	return accs[accIndex];
}

const uint8_t* GetDataPointer(const nlohmann::json& gltf, const std::vector<std::vector<uint8_t>>& buffers, const nlohmann::json& accessor)
{
	assert(accessor["bufferView"].is_number_unsigned());
	auto bufView = gltf["bufferViews"][unsigned(accessor["bufferView"])];
	assert(bufView.find("byteStride") == bufView.end()); // No support for stride for now
	return buffers[bufView["buffer"]].data() + unsigned(bufView["byteOffset"]);
}

// Make sure dst is big enough
void FillDataFromBufferForIndices(const nlohmann::json& gltf,
	const std::vector<std::vector<uint8_t>>& buffers,
	const nlohmann::json& accessor,
	uint32_t* dst)
{
	auto srcData = GetDataPointer(gltf, buffers, accessor);

	switch (unsigned(accessor["componentType"]))
	{
	case 5121: // UNSIGNED_BYTE
		std::copy(reinterpret_cast<const uint8_t*>(srcData),
			reinterpret_cast<const uint8_t*>(srcData) + accessor["count"],
			dst);
		break;
	case 5123:// UNSIGNED_SHORT
		std::copy(reinterpret_cast<const uint16_t*>(srcData),
			reinterpret_cast<const uint16_t*>(srcData) + accessor["count"],
			dst);
		break;
	default:
		assert(false);
	}
}

void FillDataForMeshVertex(const nlohmann::json& gltf,
	const std::vector<std::vector<uint8_t>>& buffersData,
	const nlohmann::json& positionAccessor,
	const nlohmann::json& normalAccessor,
	nlohmann::json* texCoordAccessor,
	Zmey::Graphics::MeshVertex* dst)
{
	assert(positionAccessor["type"] == "VEC3"
		&& normalAccessor["type"] == "VEC3"
		&& (!texCoordAccessor || texCoordAccessor->operator[]("type") == "VEC2"));
	auto positionData = reinterpret_cast<const Zmey::Vector3*>(GetDataPointer(gltf, buffersData, positionAccessor));
	auto normalData = reinterpret_cast<const Zmey::Vector3*>(GetDataPointer(gltf, buffersData, normalAccessor));
	auto texCoordData = reinterpret_cast<const Zmey::Vector2*>(texCoordAccessor ? GetDataPointer(gltf, buffersData, *texCoordAccessor) : nullptr);

	unsigned count = positionAccessor["count"];
	for (auto i = 0u; i < count; ++i)
	{
		// GLTF has Right handed system so we need to invert the Z Coordinate
		dst[i].Position = Zmey::Vector3{ positionData[i].x, positionData[i].y, -positionData[i].z };
		dst[i].Normal = Zmey::Vector3{ normalData[i].x, normalData[i].y, -normalData[i].z };
		dst[i].TextureUV = texCoordData ? texCoordData[i] : Zmey::Vector2{ 0.0f, 0.0f };

	}
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
			assert(indicesAccessor["type"].is_string());
			std::string indicesAccessorType = indicesAccessor["type"];
			assert(indicesAccessorType == "SCALAR");
			unsigned indicesSize = indicesAccessor["count"];
			indices.resize(indicesSize);
			FillDataFromBufferForIndices(gltf, buffersData, indicesAccessor, indices.data());
		}


		std::vector<Zmey::Graphics::MeshVertex> vertices;
		{
			const auto& attribs = mesh["primitives"][0]["attributes"];
			const auto& positionAccessor = FindAccessor(gltf, attribs["POSITION"]);
			const auto& normalAccessor = FindAccessor(gltf, attribs["NORMAL"]);
			bool hasTexCoord = attribs.find("TEXCOORD_0") != attribs.end();
			vertices.resize(positionAccessor["count"]);

			FillDataForMeshVertex(gltf,
				buffersData,
				positionAccessor,
				normalAccessor,
				hasTexCoord ? &FindAccessor(gltf, attribs["TEXCOORD_0"]) : nullptr,
				vertices.data());
		}

		Zmey::Graphics::MeshDataHeader data;
		data.VerticesCount = uint64_t(vertices.size());
		data.IndicesCount = uint64_t(indices.size());

		Zmey::MemoryOutputStream memstream;
		memstream.Write(reinterpret_cast<uint8_t*>(&data), sizeof(Zmey::Graphics::MeshDataHeader));
		memstream.Write(reinterpret_cast<uint8_t*>(vertices.data()), vertices.size() * sizeof(Zmey::Graphics::MeshVertex));
		memstream.Write(reinterpret_cast<uint8_t*>(indices.data()), indices.size() * sizeof(uint32_t));

		std::ofstream outputFile(destinationFolder + meshName, std::ios::binary | std::ios::out | std::ios::trunc);
		outputFile.write(reinterpret_cast<const char*>(memstream.GetData()), memstream.GetDataSize());
	}
	return true;
}
}
}
}