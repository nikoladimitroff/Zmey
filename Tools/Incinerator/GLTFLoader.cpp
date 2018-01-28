#include "GLTFLoader.h"
#include <nlohmann/json.hpp>
#include <Zmey/Graphics/Managers/MeshManager.h>
#include <Zmey/Graphics/Managers/MaterialManager.h>
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

nlohmann::json FindMaterial(const nlohmann::json& gltf, unsigned materialIndex)
{
	auto mats = gltf["materials"];
	assert(mats.is_array() && materialIndex < mats.size());
	return mats[materialIndex];
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
	const std::vector<std::string>& meshFiles,
	std::unordered_set<std::string>& additionalResources)
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

	std::unordered_set<uint16_t> materialToIncinerate;

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

		// TODO: take into account other primitives
		const auto& primitive = mesh["primitives"][0];

		std::vector<Zmey::Graphics::MeshVertex> vertices;
		{
			const auto& attribs = primitive["attributes"];
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

		uint16_t materialIndex = -1; // Default Material
		if (primitive.find("material") != primitive.end())
		{
			materialIndex = uint16_t(unsigned(primitive["material"]));
			materialToIncinerate.insert(materialIndex);
		}

		Zmey::Graphics::MeshDataHeader data;
		data.MaterialIndex = materialIndex;
		data.VerticesCount = uint64_t(vertices.size());
		data.IndicesCount = uint64_t(indices.size());

		Zmey::MemoryOutputStream memstream;
		memstream.Write(reinterpret_cast<uint8_t*>(&data), sizeof(Zmey::Graphics::MeshDataHeader));
		memstream.Write(reinterpret_cast<uint8_t*>(vertices.data()), vertices.size() * sizeof(Zmey::Graphics::MeshVertex));
		memstream.Write(reinterpret_cast<uint8_t*>(indices.data()), indices.size() * sizeof(uint32_t));

		std::ofstream outputFile(destinationFolder + meshName, std::ios::binary | std::ios::out | std::ios::trunc);
		outputFile.write(reinterpret_cast<const char*>(memstream.GetData()), memstream.GetDataSize());
	}

	for (const auto& materialIndex : materialToIncinerate)
	{
		Zmey::Graphics::MaterialDataHeader dataHeader;
		// Default data
		dataHeader.BaseColorFactor = Zmey::Color(1.0f, 1.0f, 1.0f, 1.0f);
		dataHeader.BaseColorTextureOffset = 0; // no texture
		dataHeader.BaseColorTextureSize = 0;

		uint32_t baseColorTextureIndex = -1;

		const auto& material = FindMaterial(gltf, materialIndex);
		if (material.find("pbrMetallicRoughness") != material.end())
		{
			auto pbr = material["pbrMetallicRoughness"];
			assert(pbr.is_object());
			for (const auto& member : pbr.get<nlohmann::json::object_t>())
			{
				if (member.first == "baseColorFactor")
				{
					assert(member.second.is_array());
					dataHeader.BaseColorFactor = Zmey::Color(
						member.second[0],
						member.second[1],
						member.second[2],
						member.second[3]);
				}
				else if (member.first == "baseColorTexture")
				{
					assert(member.second.is_object());
					baseColorTextureIndex = member.second["index"];
				}
			}
		}

		std::vector<uint8_t> textureData;

		// Get Textures data
		if (baseColorTextureIndex != -1)
		{
			assert(gltf["images"].is_array());
			const auto& image = gltf["images"][baseColorTextureIndex];
			assert(image["uri"].is_string());
			std::string filename = image["uri"];
			// Change file ending for now
			// TODO: read the original and make it a dds
			filename = filename.substr(0, filename.find_last_of('.'));
			filename += ".dds";

			std::ifstream imageFile(contentFolder + filename, std::ios::binary);
			imageFile.seekg(0, std::ios::end);
			size_t size = imageFile.tellg();
			imageFile.seekg(0);

			textureData.resize(size);
			imageFile.read((char*)textureData.data(), size);

			dataHeader.BaseColorTextureOffset = sizeof(Zmey::Graphics::MaterialDataHeader);
			dataHeader.BaseColorTextureSize = size;
		}

		Zmey::MemoryOutputStream memstream;
		memstream.Write(reinterpret_cast<uint8_t*>(&dataHeader), sizeof(Zmey::Graphics::MaterialDataHeader));
		memstream.Write(textureData.data(), textureData.size());

		auto fileName = std::string("material_") + std::to_string(materialIndex) + std::string(".material");
		std::ofstream outputFile(destinationFolder + fileName, std::ios::binary | std::ios::out | std::ios::trunc);
		outputFile.write(reinterpret_cast<const char*>(memstream.GetData()), memstream.GetDataSize());

		additionalResources.insert("IncineratedDataCache/" + fileName);
	}

	return true;
}
}
}
}