#pragma once
#include <Zmey/Memory/MemoryManagement.h>

struct aiScene;

namespace Zmey
{
using ResourceId = unsigned;
class ResourceLoader
{
public:
	ResourceLoader();
	~ResourceLoader();
	ResourceId LoadResource(const stl::string&);

	template<typename T>
	const T* As(ResourceId) const
	{
		ASSERT_FATAL(false && "Not supported type");
		return nullptr;
	}
	template<>
	const aiScene* As<aiScene>(ResourceId id) const
	{
		auto it = std::find_if(m_Meshes.begin(), m_Meshes.end(), [id](const std::pair<ResourceId, const aiScene*>& meshData)
		{
			return meshData.first == id;
		});
		ASSERT_RETURN_VALUE(it != m_Meshes.end(), nullptr);
		return it->second;
	}
	template<>
	const char* As<char>(ResourceId id) const
	{
		auto it = std::find_if(m_TextContents.begin(), m_TextContents.end(), [id](const std::pair<ResourceId, const stl::string>& meshData)
		{
			return meshData.first == id;
		});
		ASSERT_RETURN_VALUE(it != m_TextContents.end(), nullptr);
		return it->second.c_str();
	}
private:
	// Callback for the task system
	friend void OnResourceLoaded(ResourceLoader*, ResourceId, const aiScene*);
	friend void OnResourceLoaded(ResourceLoader*, ResourceId, const tmp::string&);

	ResourceId m_NextId;
	stl::vector<std::pair<ResourceId, const aiScene*>> m_Meshes;
	stl::vector<std::pair<ResourceId, const stl::string>> m_TextContents;
};

}