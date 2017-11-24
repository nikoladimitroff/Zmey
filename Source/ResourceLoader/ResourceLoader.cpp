#include <Zmey/ResourceLoader/ResourceLoader.h>

#include <fstream>
#include <utility>

#include <Zmey/Modules.h>
#include <Zmey/World.h>
#include <Zmey/Utilities.h>

namespace Zmey
{
ResourceLoader::ResourceLoader()
{
}

ResourceLoader::~ResourceLoader()
{
}

void ResourceLoader::ReleaseOwnershipOver(Zmey::Name name)
{
	// TODO: implement for other types
	auto it = std::find_if(m_Worlds.begin(), m_Worlds.end(), [name](const std::pair<Zmey::Name, const World*>& data)
	{
		return data.first == name;
	});
	if (it != m_Worlds.end())
	{
		// concurrent_vector doesn't support erase, so swap and resize
		// TODO Are the following 3 operations thread transactional?
		std::swap(it, m_Worlds.end() - 1);
		m_Worlds.resize(m_Worlds.size() - 1);
	}
}
#define FIRST_IN_ALL_RESOURCE_COLLECTIONS(Function, Name) \
	Function(Name, m_Meshes) || \
	Function(Name, m_TextContents) || \
	Function(Name, m_Worlds) || \
	Function(Name, m_BufferedData)

template<typename T>
bool ResourceLoader::ResourceExistsInCollection(Zmey::Name name, const stl::concurrent_vector<std::pair<Zmey::Name, T>>& collection)
{
	return FindResourceIteratorInCollection(name, collection) != collection.end();
}

bool ResourceLoader::IsResourceReady(Zmey::Name name)
{
	return FIRST_IN_ALL_RESOURCE_COLLECTIONS(ResourceExistsInCollection, name);
}

template<typename T>
bool ResourceLoader::TryFreeFromCollection(Zmey::Name name, stl::concurrent_vector<std::pair<Zmey::Name, T>>& collection)
{
	auto it = FindResourceIteratorInCollection(name, collection);
	if (it != collection.end())
	{
		auto& lastPair = collection[collection.size() - 1];
		// concurrent_vector doesn't support erase, so swap and resize
		// TODO Are the following 3 operations thread transactional?
		it->swap(lastPair);
		collection.resize(collection.size() - 1);
		return true;
	}
	return false;
}
void ResourceLoader::FreeResource(Zmey::Name name)
{
	FIRST_IN_ALL_RESOURCE_COLLECTIONS(TryFreeFromCollection, name);
}

void ResourceLoader::WaitForResource(Zmey::Name name)
{
	while (!IsResourceReady(name))
	{}
}

void ResourceLoader::WaitForAllResources(const tmp::vector<Zmey::Name>& resources)
{
	auto isResourceMissing = [this](const Zmey::Name name) { return !IsResourceReady(name); };
	while (std::any_of(resources.begin(), resources.end(), isResourceMissing))
	{}
}

void OnResourceMeshLoaded(ResourceLoader* loader, Zmey::Name name, stl::vector<uint8_t>&& data)
{
	auto handle = Modules::Renderer->MeshLoaded(std::move(data));
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

	if (IsResourceReady(name))
	{
		return name;
	}

	if (Utilities::EndsWith(path, ".typebin"))
	{
		// TODO: Add task
		{
			std::ifstream stream(path.c_str());
			ASSERT(stream.good());
			stream.seekg(0, std::ios::end);
			size_t size = stream.tellg();
			stream.seekg(0);

			stl::vector<uint8_t> data(size);
			stream.read((char*)data.data(), size);
			OnResourceLoaded(this, name, std::move(data));
		}
	}
	else if (Utilities::EndsWith(path, ".worldbin"))
	{
		// TODO: add task
		{
			std::ifstream stream(path.c_str());
			ASSERT(stream.good());
			stream.seekg(0, std::ios::end);
			size_t size = stream.tellg();
			stream.seekg(0);

			stl::unique_array<uint8_t> buffer = stl::make_unique_array<uint8_t>(size);
			stream.read((char*)buffer.get(), size);
			World* world = new World();
			world->InitializeFromBuffer(buffer.get(), size);
			OnResourceLoaded(this, name, world);
		}
	}
	else if (Utilities::EndsWith(path, ".js"))
	{
		// TODO: add task
		{
			std::ifstream stream(path.c_str());
			stream.seekg(0, std::ios::end);
			size_t size = stream.tellg();
			stream.seekg(0);
			tmp::string buffer(size, '\0');
			stream.read(&buffer[0], size);
			OnResourceLoaded(this, name, buffer);
		}
	}
	else
	{
		// TODO: add task
		{
			// TODO: maybe temp memory ?
			std::ifstream stream(path.c_str());
			ASSERT(stream.good());
			stream.seekg(0, std::ios::end);
			size_t size = stream.tellg();
			stream.seekg(0);

			stl::vector<uint8_t> data(size);
			stream.read((char*)data.data(), size);
			OnResourceMeshLoaded(this, name, std::move(data));
		}
	}
	return name;
}

}

#undef FIRST_IN_ALL_RESOURCE_COLLECTIONS