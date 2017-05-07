#include <Zmey/ResourceLoader/ResourceLoader.h>

// Use the C interface as the CPP interface at let's us destroy the aiScene when we decide we want to
// and is also thread-safe
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/LogStream.hpp>
#include <assimp/DefaultLogger.hpp>

#include <Zmey/Modules.h>

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

void OnResourceLoaded(ResourceLoader* loader, ResourceId id, const aiScene* scene)
{
	loader->m_Meshes.push_back(std::make_pair(id, scene));
}

ResourceId ResourceLoader::LoadResource(const stl::string& path)
{
	auto id = m_NextId++;
	Modules::TaskSystem->SpawnTask("Loading file", [&path, this, id]()
	{
		const aiScene* scene = aiImportFile(path.c_str(), aiPostProcessSteps::aiProcess_ValidateDataStructure);
		OnResourceLoaded(this, id, scene);
	});
	return id;
}

}