#include "Incinerator.h"

#include <iostream>
#include <fstream>
#include <vector>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <Zmey/Components/ComponentRegistry.h>
#include <nlohmann/json.hpp>


namespace
{
std::vector<std::string> FindAllFiles(const std::string& directory, const std::string& extension)
{
	const std::string searchString = directory + extension;
	WIN32_FIND_DATA ffd;
	auto findHandle = ::FindFirstFile(searchString.c_str(), &ffd);
	if (INVALID_HANDLE_VALUE == findHandle)
	{
		std::cerr << "Failed to find shaders to compile!" << std::endl;
		return {};
	}

	std::vector<std::string> fileList;
	do
	{
		fileList.push_back(directory + ffd.cFileName);
	} while (::FindNextFile(findHandle, &ffd) != 0);
	::FindClose(findHandle);

	return fileList;
}

// Operator to dump binary data into any ostream
template <typename T, class... StreamArgs>
inline std::basic_ostream<StreamArgs...>& operator <=(std::basic_ostream<StreamArgs...> & out, T const & data) {
	out.write(reinterpret_cast<char const *>(&data), sizeof(T));
	return out;
}

}

void Incinerator::Incinerate(const Options& options)
{
	const std::string contentDir = options.GameDirectory + "/Content/";
	const std::string compiledDir = options.GameDirectory + "/IncineratedDataCache/";
	const std::string worldExtension = "*.world";
	const std::string classExtension = "*.type";
	const std::string binaryExtension = ".bin";

	auto classDescriptors = FindAllFiles(contentDir, classExtension);
	BuildClassIndex(classDescriptors);

	auto worlds = FindAllFiles(contentDir, worldExtension);
	for (const auto& worldSectionFile : worlds)
	{
		IncinerateWorld(compiledDir, worldSectionFile);
	}
}

void Incinerator::BuildClassIndex(const std::vector<std::string>& classFiles)
{
	for(const std::string& filePath : classFiles)
	{
		nlohmann::json rawJson;
		std::ifstream file(filePath);
		file >> rawJson;

		std::string version = rawJson["version"];
		assert(std::strcmp(version.c_str(), "1.0") == 0);

		std::string className = rawJson["name"];
		std::string parentClass = rawJson["extends"];
		assert(parentClass.size() == 0); // TODO support inheritance

		ClassEntry classEntry;
		std::vector<nlohmann::json> componentList = rawJson["components"];
		for (const auto& rawComponentData : componentList)
		{
			auto name = rawComponentData["name"].get<std::string>();
			ComponentEntry binaryComponentData(name);
			Zmey::Hash nameHash(name.c_str());
			auto compiler = Zmey::Components::GetComponentManager(nameHash);
			compiler.ToBlob(rawComponentData, binaryComponentData);

			classEntry.Components.push_back(binaryComponentData);
		}
		m_ClassIndex[className] = classEntry;
	}
}

void Incinerator::ComponentEntry::WriteData(Zmey::Hash nameHash, const uint8_t* data, uint16_t dataSize)
{
	SingleDataValue value;
	value.resize(dataSize);
	std::memcpy(value.data(), data, dataSize);

	Data.insert_or_assign(nameHash, std::move(value));
}

void Incinerator::IncinerateWorld(const std::string& destinationFolder, const std::string& worldSectionPath)
{
	std::ifstream worldFile(worldSectionPath);
	nlohmann::json rawData;
	worldFile >> rawData;

	std::stringstream membuffer;
	membuffer << "1.0"; // Version
						// Go through all entities
	assert(rawData.count("entities") != 0);
	struct EntityDataPerComponent
	{
		Zmey::EntityId::IndexType EntityIndex;
		ComponentEntry Data;
	};
	std::unordered_map<std::string, std::vector<EntityDataPerComponent>> entitiesForComponent;
	Zmey::EntityId::IndexType maxEntityIndex = 0u;

	for (const auto& entityData : rawData["entities"])
	{
		// Go through each entity and serialize their components
		const std::string entityClass = entityData["type"];
		const auto entityIndex = maxEntityIndex++;
		auto& classEntry = m_ClassIndex[entityClass];
		for (const auto& inheritedComponent : classEntry.Components)
		{
			auto& entities = entitiesForComponent[inheritedComponent.ComponentName];
			entities.push_back({ entityIndex, inheritedComponent });
		}
		for (const auto& newComponent : entityData["components"])
		{
			const std::string componentName = newComponent["name"];
			auto& entities = entitiesForComponent[componentName];
			// If this overrides an inherited components, this entity must be the last one added
			bool overridesInherited = entities[entities.size() - 1].EntityIndex == entityIndex;
			if (!overridesInherited)
			{
				entities.push_back({ entityIndex, ComponentEntry(componentName) });
			}
			auto& componentEntry = entities[entities.size() - 1].Data;

			Zmey::Hash componentNameHash(componentName.c_str());
			auto compiler = Zmey::Components::GetComponentManager(componentNameHash);
			compiler.ToBlob(newComponent, componentEntry);
		}
	}

	membuffer << maxEntityIndex;
	for (const auto& it : entitiesForComponent)
	{
		membuffer << it.first.c_str();
		membuffer << it.second.size();
		for (const auto& entityData : it.second)
		{
			membuffer << entityData.EntityIndex;
		}
		for (const auto& entityData : it.second)
		{
			for (const auto& entityDataPiece : entityData.Data.Data)
			{
				auto dataPtr = reinterpret_cast<const char*>(entityDataPiece.second.data());
				auto dataSize = entityDataPiece.second.size();
				membuffer.write(dataPtr, dataSize);
			}
		}
	}

	std::ofstream outputFile(destinationFolder + "/testworld.bin", std::ios::binary | std::ios::out);
	outputFile << membuffer.rdbuf();
}