#pragma once
#include <Zmey/Memory/MemoryManagement.h>
#include <Zmey/Hash.h>
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

class ResourceLoader
{
	template<typename T>
	typename stl::concurrent_vector<std::pair<Zmey::Name, T>>::iterator
		FindResourceIteratorInCollection(Zmey::Name name, stl::concurrent_vector<std::pair<Zmey::Name, T>>& collection) const
	{
		auto it = std::find_if(collection.begin(), collection.end(), [name](const std::pair<Zmey::Name, T>& data)
		{
			return data.first == name;
		});
		return it;
	}
	template<typename T>
	typename stl::concurrent_vector<std::pair<Zmey::Name, T>>::const_iterator
		FindResourceIteratorInCollection(Zmey::Name name, const stl::concurrent_vector<std::pair<Zmey::Name, T>>& collection) const
	{
		auto it = std::find_if(collection.cbegin(), collection.cend(), [name](const std::pair<Zmey::Name, T>& data)
		{
			return data.first == name;
		});
		return it;
	}
	template<typename T>
	const T* FindResourceInCollection(Zmey::Name name,
		const stl::concurrent_vector<std::pair<Zmey::Name, T>>& collection) const
	{
		auto it = FindResourceIteratorInCollection(name, collection);
		if (it != collection.end())
		{
			return &it->second;
		}
		return nullptr;
	}
public:
	ResourceLoader();
	~ResourceLoader();

	ResourceLoader(const ResourceLoader&) = delete;
	ResourceLoader(ResourceLoader&&) = delete;
	ResourceLoader& operator=(const ResourceLoader&) = delete;
	// Starts loading the resource at the given path.
	// @return The name of the resource which can also be acquired via Zmey::Name::Name()
	// but this function returns it for easier usage.
	ZMEY_API Zmey::Name LoadResource(const stl::string& path);
	ZMEY_API bool IsResourceReady(Zmey::Name pathHash);
	ZMEY_API void WaitForResource(Zmey::Name name);
	ZMEY_API void WaitForAllResources(const tmp::vector<Zmey::Name>& resources);

	// Use non-template methods for public access so as the client doesn't have to wonder
	// what exact type should he pass in the templated function.

	ZMEY_API const Graphics::MeshHandle* AsMeshHandle(Zmey::Name name) const
	{
		return FindResourceInCollection(name, m_Meshes);
	}
	ZMEY_API const World* AsWorld(Zmey::Name name) const
	{
		World* const* world = FindResourceInCollection(name, m_Worlds);
		return world ? *world : nullptr;
	}
	ZMEY_API const stl::vector<uint8_t>* AsBuffer(Zmey::Name name) const
	{
		return FindResourceInCollection(name, m_BufferedData);
	}
	ZMEY_API const stl::string* AsText(Zmey::Name name) const
	{
		return FindResourceInCollection(name, m_TextContents);
	}
	ZMEY_API void ReleaseOwnershipOver(Zmey::Name);
	ZMEY_API void FreeResource(Zmey::Name name);
private:
	template<typename T>
	bool TryFreeFromCollection(Zmey::Name name, stl::concurrent_vector<std::pair<Zmey::Name, T>>& collection);
	template<typename T>
	bool ResourceExistsInCollection(Zmey::Name name, const stl::concurrent_vector<std::pair<Zmey::Name, T>>& collection);
	// Callback for the task system
	friend void OnResourceLoaded(ResourceLoader*, Zmey::Name, const aiScene*);
	friend void OnResourceLoaded(ResourceLoader*, Zmey::Name, const tmp::string&);
	friend void OnResourceLoaded(ResourceLoader*, Zmey::Name, World*);
	friend void OnResourceLoaded(ResourceLoader*, Zmey::Name, stl::vector<uint8_t>&&);

	stl::concurrent_vector<std::pair<Zmey::Name, Graphics::MeshHandle>> m_Meshes;
	stl::concurrent_vector<std::pair<Zmey::Name, stl::string>> m_TextContents;
	stl::concurrent_vector<std::pair<Zmey::Name, World*>> m_Worlds;
	stl::concurrent_vector<std::pair<Zmey::Name, stl::vector<uint8_t>>> m_BufferedData;
};

}