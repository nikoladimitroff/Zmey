#include <Zmey/ResourceLoader/ResourceLoader.h>

#include <fstream>
// Use the C interface as the CPP interface at let's us destroy the aiScene when we decide we want to
// and is also thread-safe
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/LogStream.hpp>
#include <assimp/DefaultLogger.hpp>

#include <Zmey/Modules.h>
#include <Zmey/World.h>

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

void ResourceLoader::ReleaseOwnershipOver(ResourceId id)
{
	// TODO: implement for other types
	auto it = std::find_if(m_Worlds.begin(), m_Worlds.end(), [id](const std::pair<ResourceId, const World*>& data)
	{
		return data.first == id;
	});
	if (it != m_Worlds.end())
	{
		m_Worlds.erase(it);
	}
}

bool ResourceLoader::IsResourceReady(ResourceId id)
{
	bool found = false;
	auto it = std::find_if(m_Meshes.begin(), m_Meshes.end(), [id](const std::pair<ResourceId, const aiScene*>& meshData)
	{
		return meshData.first == id;
	});
	found |= it != m_Meshes.end();
	auto it2 = std::find_if(m_TextContents.begin(), m_TextContents.end(), [id](const std::pair<ResourceId, const stl::string>& meshData)
	{
		return meshData.first == id;
	});
	found |= it2 != m_TextContents.end();
	auto it3 = std::find_if(m_Worlds.begin(), m_Worlds.end(), [id](const std::pair<ResourceId, const World*>& data)
	{
		return data.first == id;
	});
	found |= it3 != m_Worlds.end();
	return found;
}

void OnResourceLoaded(ResourceLoader* loader, ResourceId id, const aiScene* scene)
{
	loader->m_Meshes.push_back(std::make_pair(id, scene));
	FORMAT_LOG(Info, ResourceLoader, "Just loaded asset for id: %d", id);
}
void OnResourceLoaded(ResourceLoader* loader, ResourceId id, const tmp::string& text)
{
	loader->m_TextContents.push_back(std::make_pair(id, text.c_str()));
	FORMAT_LOG(Info, ResourceLoader, "Just loaded asset for id: %d", id);
}
void OnResourceLoaded(ResourceLoader* loader, ResourceId id, const World* world)
{
	loader->m_Worlds.push_back(std::make_pair(id, world));
	FORMAT_LOG(Info, ResourceLoader, "Just loaded asset for id: %d", id);
}

inline bool EndsWith(const stl::string& value, const stl::string&ending)
{
	if (ending.size() > value.size()) return false;
	return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

ResourceId ResourceLoader::LoadResource(const stl::string& path)
{
	auto id = m_NextId++;
	if (EndsWith(path, ".world"))
	{
		Modules::TaskSystem->SpawnTask("Loading file", [path, this, id]()
		{
			std::ifstream stream(path.c_str());
			stream.seekg(0, std::ios::end);
			size_t size = stream.tellg();
			stream.seekg(0);
			stl::unique_array<uint8_t> buffer = stl::make_unique_array<uint8_t>(size);
			stream.read((char*)buffer.get(), size);
			World* world = new World();
			world->InitializeFromBuffer(buffer.get(), size);
			OnResourceLoaded(this, id, world);
		});
		return id;
	}
	if (EndsWith(path, ".js"))
	{
		Modules::TaskSystem->SpawnTask("Loading file", [path, this, id]()
		{
			std::ifstream stream(path.c_str());
			stream.seekg(0, std::ios::end);
			size_t size = stream.tellg();
			stream.seekg(0);
			tmp::string buffer(size, '\0');
			stream.read(&buffer[0], size);
			OnResourceLoaded(this, id, buffer);
		});
		return id;
	}
	Modules::TaskSystem->SpawnTask("Loading file", [path, this, id]()
	{
		const aiScene* scene = aiImportFile(path.c_str(), aiPostProcessSteps::aiProcess_ValidateDataStructure);
		OnResourceLoaded(this, id, scene);
	});
	return id;
}

}