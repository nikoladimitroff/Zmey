#pragma once
#include <Zmey/Config.h>
#include <Zmey/EntityManager.h>
#include <Zmey/Memory/MemoryManagement.h>
#include <Zmey/Hash.h>
#include <Zmey/Components/ComponentRegistryCommon.h>

#include <nlohmann/json.hpp> // TODO: remove this include as it will clutter everything

namespace Zmey
{

namespace Components
{
	class IDataBlob
	{
	public:
		virtual ~IDataBlob() {}
		virtual void WriteData(Hash dataName, const uint8_t* data, uint16_t dataSize) = 0;
	};
	struct ComponentManagerEntry
	{
		using InstantiateDelegate = void* (*)();
		using DestroyDelegate = void (*)(void*);
		using ToBlobDelegate = void (*)(const nlohmann::json&, IDataBlob& blob);
		using FromBlobDelegate = void (*)(const tmp::vector<EntityId>& entities, const char* blob);

		ComponentManagerEntry(const char* name, ComponentIndex ComponentManagerIndex, InstantiateDelegate, DestroyDelegate, ToBlobDelegate, FromBlobDelegate);
		const Hash Name;
		InstantiateDelegate Instantiate;
		DestroyDelegate Destroy;
		const ToBlobDelegate ToBlob;
		const FromBlobDelegate FromBlob;
	};

	ZMEY_API ComponentIndex GetNextComponentManagerIndex();
	ZMEY_API const ComponentManagerEntry& GetComponentManager(Hash nameHash);
	ZMEY_API const ComponentManagerEntry* GetComponentManagerAtIndex(ComponentIndex);

	template<typename T>
	void* InstantiateManager()
	{
		return new T();
	}
	template<typename T>
	void DestroyManager(void* ptr)
	{
		delete ptr;
	}
#define DEFINE_COMPONENT_MANAGER(Class, ShortName, ToBlob, FromBlob) \
	const ComponentIndex Class##::SZmeyComponentManagerIndex = GetNextComponentManagerIndex(); \
	static Zmey::Components::ComponentManagerEntry G##ShortName##ComponentManagerRegistration(#ShortName, \
		Class##::SZmeyComponentManagerIndex, \
		&InstantiateManager<##Class##>, &DestroyManager<##Class##>, \
		ToBlob, FromBlob)
}

}