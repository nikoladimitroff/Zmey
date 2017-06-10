#include <Zmey/ResourceLoader/ResourceLoader.h>

#include <fstream>
#include <utility>
// Use the C interface as the CPP interface at let's us destroy the aiScene when we decide we want to
// and is also thread-safe
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/LogStream.hpp>
#include <assimp/DefaultLogger.hpp>

#include <Zmey/Modules.h>
#include <Zmey/World.h>
#include <Zmey/Utilities.h>

namespace Zmey
{
class AssimpLogStream :	public Assimp::LogStream
{
public:
	// Write womethink using your own functionality
	virtual void write(const char* message) override
	{
		LOG(Info, Assimp, message);
	}
};

ResourceLoader::ResourceLoader()
{
	Assimp::DefaultLogger::create("", Assimp::Logger::VERBOSE);
	Assimp::DefaultLogger::get()->attachStream(new AssimpLogStream(), Assimp::Logger::Info);
}

ResourceLoader::~ResourceLoader()
{
	Assimp::DefaultLogger::kill();
}

void ResourceLoader::ReleaseOwnershipOver(Zmey::Name name)
{
	//auto it = m_Resources.find(name);
	//if (it != m_Resources.end())
	//{
	//	m_Resources.erase(it);
	//}
}

template<typename T>
bool ResourceLoader::ResourceExistsInCollection(Zmey::Name name, stl::vector<std::pair<Zmey::Name, T>> collection)
{
	return FindResourceIteratorInCollection(name, collection) != collection.end();
}

bool ResourceLoader::IsResourceReady(Zmey::Name name)
{
	return ResourceExistsInCollection(name, m_Meshes) ||
		ResourceExistsInCollection(name, m_TextContents) ||
		ResourceExistsInCollection(name, m_Worlds) ||
		ResourceExistsInCollection(name, m_BufferedData);
}

template<typename T>
bool ResourceLoader::TryFreeFromCollection(Zmey::Name name, stl::vector<std::pair<Zmey::Name, T>> collection)
{
	auto it = FindResourceIteratorInCollection(name, collection);
	if (it != collection.end())
	{
		auto& lastPair = collection[collection.size() - 1];
		it->swap(lastPair);
		collection.pop_back();
		return true;
	}
	return false;
}
void ResourceLoader::FreeResource(Zmey::Name name)
{
	TryFreeFromCollection(name, m_Meshes) ||
		TryFreeFromCollection(name, m_TextContents) ||
		TryFreeFromCollection(name, m_Worlds) ||
		TryFreeFromCollection(name, m_BufferedData);
}

void OnResourceLoaded(ResourceLoader* loader, Zmey::Name name, const aiScene* scene)
{
	auto handle = Modules::Renderer->MeshLoaded(scene);
	loader->m_Meshes.push_back(std::make_pair(name, handle));
	FORMAT_LOG(Info, ResourceLoader, "Just loaded asset for name: %llu", static_cast<uint64_t>(name));
}
void OnResourceLoaded(ResourceLoader* loader, Zmey::Name name, const tmp::string& text)
{
	loader->m_TextContents.push_back(std::make_pair(name, text.c_str()));
	FORMAT_LOG(Info, ResourceLoader, "Just loaded asset for name: %llu", static_cast<uint64_t>(name));
}
void OnResourceLoaded(ResourceLoader* loader, Zmey::Name name, World* world)
{
	loader->m_Worlds.push_back(std::make_pair(name, world));
	FORMAT_LOG(Info, ResourceLoader, "Just loaded asset for name: %llu", static_cast<uint64_t>(name));
}
void OnResourceLoaded(ResourceLoader* loader, Zmey::Name name, stl::vector<uint8_t>&& data)
{
	loader->m_BufferedData.push_back(std::make_pair(name, std::move(data)));
	FORMAT_LOG(Info, ResourceLoader, "Just loaded asset for name: %llu", static_cast<uint64_t>(name));
}

Zmey::Name ResourceLoader::LoadResource(const stl::string& path)
{
	const Zmey::Name name(path.c_str());
	if (Utilities::EndsWith(path, ".bin"))
	{
		Modules::TaskSystem->SpawnTask("Loading file", [path, this, name]()
		{
			std::ifstream stream(path.c_str());
			stream.seekg(0, std::ios::end);
			size_t size = stream.tellg();
			stream.seekg(0);
			BinaryFileTypes markerType;
			stream >> (char&) markerType;
			size--; // Take 1 for the markerType

			stl::unique_array<uint8_t> buffer = stl::make_unique_array<uint8_t>(size);
			stream.read((char*)buffer.get(), size);
			if (markerType == BinaryFileTypes::ClassDescriptor)
			{
				stl::vector<uint8_t> data(size);
				std::memcpy(&data[0], buffer.get(), size);
				OnResourceLoaded(this, name, std::move(data));
			}
			else if (markerType == BinaryFileTypes::WorldSection)
			{
				World* world = new World();
				world->InitializeFromBuffer(buffer.get(), size);
				OnResourceLoaded(this, name, world);
			}
			else
			{
				NOT_REACHED();
			}
		});
	}
	else if (Utilities::EndsWith(path, ".js"))
	{
		Modules::TaskSystem->SpawnTask("Loading file", [path, this, name]()
		{
			std::ifstream stream(path.c_str());
			stream.seekg(0, std::ios::end);
			size_t size = stream.tellg();
			stream.seekg(0);
			tmp::string buffer(size, '\0');
			stream.read(&buffer[0], size);
			OnResourceLoaded(this, name, buffer);
		});
	}
	else
	{
		Modules::TaskSystem->SpawnTask("Loading file", [path, this, name]()
		{
			const aiScene* scene = aiImportFile(path.c_str(), aiPostProcessSteps::aiProcess_ValidateDataStructure | aiProcess_MakeLeftHanded | aiProcess_FlipUVs | aiProcess_Triangulate);
			if (scene)
			{
				OnResourceLoaded(this, name, scene);
			}
		});
	}
	return name;
}

}