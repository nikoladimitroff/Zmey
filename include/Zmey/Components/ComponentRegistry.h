#pragma once
#include <Zmey/Config.h>
#include <Zmey/EntityManager.h>
#include <Zmey/Memory/MemoryManagement.h>
#include <Zmey/Hash.h>
#include <Zmey/Components/ComponentRegistryCommon.h>
#include <Zmey/Components/ComponentManager.h>

#include <nlohmann/json.hpp> // TODO: remove this include as it will clutter everything

namespace Zmey
{
class World;
namespace Components
{
	class IDataBlob
	{
	public:
		virtual ~IDataBlob() {}
		virtual void WriteData(Hash dataName, const uint8_t* data, uint16_t dataSize) = 0;
		virtual void WriteData(Hash dataName, const char* text, uint16_t textSize) = 0;
		virtual void RequestResource(const char* resourcePath, uint16_t size) = 0;
	};
	struct ComponentManagerEntry
	{
		using InstantiateDelegate = ComponentManager* (*)(World&);
		using DefaultsToBlobDelegate = void(*)(IDataBlob& blob);
		using ToBlobDelegate = void (*)(const nlohmann::json&, IDataBlob& blob);

		ComponentManagerEntry(const char* fullName, const char* shortName, ComponentIndex componentManagerIndex, InstantiateDelegate, DefaultsToBlobDelegate, ToBlobDelegate);
		const char* FullName;
		const char* ShortName;
		const Hash ShortNameHash;
		const ComponentIndex Index;
		const InstantiateDelegate Instantiate;
		const DefaultsToBlobDelegate DefaultsToBlob;
		const ToBlobDelegate ToBlob;
	};
	void ExportComponentsToScripting();

	void EmptyDefaultsToBlobImplementation(IDataBlob& blob);
	void EmptyToBlobImplementation(const nlohmann::json&, IDataBlob& blob);

	ZMEY_API ComponentIndex GetNextComponentManagerIndex();
	ZMEY_API const ComponentManagerEntry& GetComponentManager(Hash nameHash);
	ZMEY_API const ComponentManagerEntry* GetComponentManagerAtIndex(ComponentIndex);

	template<typename T>
	ComponentManager* InstantiateManager(World& world)
	{
		return new T(world);
	}

#define DEFINE_COMPONENT_MANAGER(Class, ShortName, DefaultsToBlob, ToBlob) \
	ZMEY_API const ComponentIndex Class##::SZmeyComponentManagerIndex = GetNextComponentManagerIndex(); \
	static Zmey::Components::ComponentManagerEntry G##ShortName##ComponentManagerRegistration(#Class, #ShortName, \
		Class##::SZmeyComponentManagerIndex, \
		&InstantiateManager<##Class##>, \
		DefaultsToBlob, \
		ToBlob)
}

}