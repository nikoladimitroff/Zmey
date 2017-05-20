#pragma once
#include <Zmey/Memory/MemoryManagement.h>

struct aiScene;

namespace Zmey
{
class World;
using ResourceId = unsigned;
class ResourceLoader
{
public:
	ResourceLoader();
	~ResourceLoader();
	ResourceId LoadResource(const stl::string&);
	bool IsResourceReady(ResourceId);

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
	template<>
	const World* As<World>(ResourceId id) const
	{
		auto it = std::find_if(m_Worlds.begin(), m_Worlds.end(), [id](const std::pair<ResourceId, const World*>& data)
		{
			return data.first == id;
		});
		ASSERT_RETURN_VALUE(it != m_Worlds.end(), nullptr);
		return it->second;
	}
	template<typename T>
	T* TakeOwnershipOver(ResourceId id)
	{
		const T* resource = As<T>(id);
		ReleaseOwnershipOver(id);
		return const_cast<T*>(resource);
	}
private:
	void ReleaseOwnershipOver(ResourceId);
	// Callback for the task system
	friend void OnResourceLoaded(ResourceLoader*, ResourceId, const aiScene*);
	friend void OnResourceLoaded(ResourceLoader*, ResourceId, const tmp::string&);
	friend void OnResourceLoaded(ResourceLoader*, ResourceId, const World*);

	ResourceId m_NextId;
	stl::vector<std::pair<ResourceId, const aiScene*>> m_Meshes;
	stl::vector<std::pair<ResourceId, const stl::string>> m_TextContents;
	stl::vector<std::pair<ResourceId, const World*>> m_Worlds;
};

}