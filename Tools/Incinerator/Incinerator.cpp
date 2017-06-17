#include "Incinerator.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_set>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <Zmey/Components/ComponentRegistry.h>
#include <Zmey/MemoryStream.h>
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
		std::cerr << "Failed to find " << extension << " files to compile!" << std::endl;
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
	for (const auto& classDescriptorFile : classDescriptors)
	{
		IncinerateClass(compiledDir, "Magician");
	}

	auto mkdirResult = ::CreateDirectory(compiledDir.c_str(), NULL);
	assert(mkdirResult != ERROR_PATH_NOT_FOUND);

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
			Zmey::Hash nameHash(Zmey::HashHelpers::CaseInsensitiveStringWrapper(name.c_str()));
			auto compiler = Zmey::Components::GetComponentManager(nameHash);
			compiler.DefaultsToBlob(binaryComponentData);
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

	if (PropertyData.find(nameHash) == PropertyData.end())
	{
		PropertyInsertionOrder.push_back(nameHash);
	}
	PropertyData.insert_or_assign(nameHash, std::move(value));
}

void Incinerator::ComponentEntry::WriteData(Zmey::Hash nameHash, const char* text, uint16_t textSize)
{
	SingleDataValue value;
	value.resize(textSize);
	std::memcpy(value.data(), text, textSize);
	value.push_back(0); // terminating zero

	if (PropertyData.find(nameHash) == PropertyData.end())
	{
		PropertyInsertionOrder.push_back(nameHash);
	}
	PropertyData.insert_or_assign(nameHash, std::move(value));
}

void Incinerator::ComponentEntry::RequestResource(const char* resourcePath, uint16_t size)
{
	Resources.push_back(std::string(resourcePath, size));
}

void Incinerator::IncinerateClass(const std::string& destinationFolder, const std::string& className)
{
	Zmey::MemoryOutputStream memstream;
	memstream << (char)0; // 0 for class descriptor marker; TODO REPLACE WITH ENUM
	memstream << "1.0"; // Version
	Zmey::Hash classNameHash(Zmey::HashHelpers::CaseInsensitiveStringWrapper(className.c_str()));
	memstream << static_cast<uint64_t>(classNameHash);
	for (const auto& fullComponentInfo : m_ClassIndex[className].Components)
	{
		Zmey::Hash componentNameHash(Zmey::HashHelpers::CaseInsensitiveStringWrapper(fullComponentInfo.ComponentName.c_str()));
		memstream << static_cast<uint64_t>(componentNameHash);

		size_t propertiesCountForComponent = fullComponentInfo.PropertyData.size();
		// Iterate over all properties for this component
		for (size_t i = 0; i < propertiesCountForComponent; i++)
		{
			// Write the data for each property, resulting in all data of the same property laid sequentially in memory
			Zmey::Hash propertyHash = fullComponentInfo.PropertyInsertionOrder[i];
			auto propertyIterator = fullComponentInfo.PropertyData.find(propertyHash);
			assert(propertyIterator != fullComponentInfo.PropertyData.end());
			const uint8_t* dataPtr = propertyIterator->second.data();
			size_t dataSize = propertyIterator->second.size();
			memstream.Write(dataPtr, dataSize);
		}
	}
	std::ofstream outputFile(destinationFolder + "/TestClass.bin", std::ios::binary | std::ios::out | std::ios::trunc);
	outputFile.write(reinterpret_cast<const char*>(memstream.GetData()), memstream.GetDataSize());
}

void Incinerator::IncinerateWorld(const std::string& destinationFolder, const std::string& worldSectionPath)
{
	std::ifstream worldFile(worldSectionPath);
	nlohmann::json rawData;
	worldFile >> rawData;

	// Go through all entities
	assert(rawData.count("entities") != 0);
	struct EntityDataPerComponent
	{
		Zmey::EntityId::IndexType EntityIndex;
		ComponentEntry DataForComponent;
	};
	std::unordered_map<std::string, std::vector<EntityDataPerComponent>> entitiesForComponent;
	Zmey::EntityId::IndexType maxEntityIndex = 0u;

	std::unordered_set<std::string> resourceList;
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
		for (const auto& currentComponentData : entityData["components"])
		{
			const std::string componentName = currentComponentData["name"];
			auto& entities = entitiesForComponent[componentName];
			// If this overrides an inherited components, this entity must be the last one added // srsly?
			bool overridesInherited = entities.size() != 0 && entities[entities.size() - 1].EntityIndex == entityIndex;
			if (!overridesInherited)
			{
				entities.push_back({ entityIndex, ComponentEntry(componentName) });
			}
			auto& componentEntry = entities[entities.size() - 1].DataForComponent;

			// Request data from the component manager
			Zmey::Hash componentNameHash(Zmey::HashHelpers::CaseInsensitiveStringWrapper(componentName.c_str()));
			auto compiler = Zmey::Components::GetComponentManager(componentNameHash);
			if (!overridesInherited)
			{
				compiler.DefaultsToBlob(componentEntry);
			}
			compiler.ToBlob(currentComponentData, componentEntry);

			// Pick up the resources that the component manager required
			std::copy(componentEntry.Resources.cbegin(), componentEntry.Resources.cend(),
				std::inserter(resourceList, resourceList.begin()));
		}
	}
	// Add resources required by default type instances
	for (const auto& classInfo : m_ClassIndex)
	{
		for (const auto& componentData : classInfo.second.Components)
		{
			std::copy(componentData.Resources.cbegin(), componentData.Resources.cend(),
				std::inserter(resourceList, resourceList.begin()));
		}
	}

	// Serialization time
	Zmey::MemoryOutputStream memstream;
	memstream << (char)1; // 1 for world marker; TODO REPLACE WITH ENUM
	memstream << "1.0"; // Version

	// Resources
	memstream << (uint64_t) resourceList.size();
	for (const auto& name : resourceList)
	{
		memstream << name;
	}

	// Entities
	memstream << maxEntityIndex;
	for (const auto& fullComponentInfo : entitiesForComponent)
	{
		Zmey::Hash componentNameHash(fullComponentInfo.first.c_str());
		memstream << static_cast<uint64_t>(componentNameHash);
		memstream << static_cast<Zmey::EntityId::IndexType>(fullComponentInfo.second.size());
		for (const auto& entityData : fullComponentInfo.second)
		{
			memstream << entityData.EntityIndex;
		}
		// Assume that the data for each instance of the same component has the same structure
		auto& firstEntityInfo = fullComponentInfo.second[0];
		size_t propertiesCountForComponent = firstEntityInfo.DataForComponent.PropertyData.size();
		// Iterate over all properties for this component
		for (size_t i = 0; i < propertiesCountForComponent; i++)
		{
			// Iterate over all entities that have this component
			for (const EntityDataPerComponent& entityData : fullComponentInfo.second)
			{
				// Write the data for each property, resulting in all data of the same property laid sequentially in memory
				Zmey::Hash propertyHash = entityData.DataForComponent.PropertyInsertionOrder[i];
				auto propertyIterator = entityData.DataForComponent.PropertyData.find(propertyHash);
				assert(propertyIterator != entityData.DataForComponent.PropertyData.end());
				const uint8_t* dataPtr = propertyIterator->second.data();
				size_t dataSize = propertyIterator->second.size();
				memstream.Write(dataPtr, dataSize);
			}
		}
	}

	std::ofstream outputFile(destinationFolder + "/testworld.bin", std::ios::binary | std::ios::out | std::ios::trunc);
	outputFile.write(reinterpret_cast<const char*>(memstream.GetData()), memstream.GetDataSize());
}