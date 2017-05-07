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
		static_assert("Not supported type");
	}
	template<>
	const aiScene* As(ResourceId id) const
	{
		auto it = std::find_if(m_Meshes.begin(), m_Meshes.end(), [id](const std::pair<ResourceId, const aiScene*>& meshData)
		{
			return meshData.first == id;
		});
		ASSERT_RETURN_VALUE(it != m_Meshes.end(), nullptr);
		return it->second;
	}
private:
	// Callback for the task system
	friend void OnResourceLoaded(ResourceLoader*, ResourceId, const aiScene*);

	ResourceId m_NextId;
	stl::vector<std::pair<ResourceId, const aiScene*>> m_Meshes;
};

}