#pragma once
#include <Zmey/Memory/MemoryManagement.h>
#include <Zmey/Graphics/GraphicsObjects.h>

struct aiScene;

namespace Zmey
{
enum class BinaryFileTypes : uint8_t
{
	ClassDescriptor,
	WorldSection
};
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
	const Graphics::MeshHandle* As<Graphics::MeshHandle>(ResourceId id) const
	{
		auto it = std::find_if(m_Meshes.begin(), m_Meshes.end(), [id](const std::pair<ResourceId, Graphics::MeshHandle>& meshData)
		{
			return meshData.first == id;
		});
		ASSERT_RETURN_VALUE(it != m_Meshes.end(), nullptr);
		return &it->second;
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
	template<>
	const stl::vector<uint8_t>* As<stl::vector<uint8_t>>(ResourceId id) const
	{
		auto it = std::find_if(m_BufferedData.begin(), m_BufferedData.end(), [id](const std::pair<ResourceId, const stl::vector<uint8_t>>& data)
		{
			return data.first == id;
		});
		ASSERT_RETURN_VALUE(it != m_BufferedData.end(), nullptr);
		return &it->second;
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
	friend void OnResourceLoaded(ResourceLoader*, ResourceId, const stl::vector<uint8_t>&);

	ResourceId m_NextId;
	stl::vector<std::pair<ResourceId, Graphics::MeshHandle>> m_Meshes;
	stl::vector<std::pair<ResourceId, const stl::string>> m_TextContents;
	stl::vector<std::pair<ResourceId, const World*>> m_Worlds;
	stl::vector<std::pair<ResourceId, const stl::vector<uint8_t>>> m_BufferedData;
};

}