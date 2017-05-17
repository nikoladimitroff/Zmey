#pragma once
#include <Zmey/Config.h>
#include <Zmey/EntityManager.h>
#include <Zmey/Memory/MemoryManagement.h>
#include <Zmey/Hash.h>

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
	struct ComponentCompiler
	{
		using ToBlobDelegate = void (*)(const nlohmann::json&, IDataBlob& blob);
		using FromBlobDelegate = void (*)(const tmp::vector<EntityId>& entities, const char* blob);

		ComponentCompiler(const char* name, ToBlobDelegate toBlob, FromBlobDelegate fromBlob);
		const Hash Name;
		const ToBlobDelegate ToBlob;
		const FromBlobDelegate FromBlob;
	};

#define REGISTER_COMPONENT_MANAGER(Name, ToBlob, FromBlob) \
	static Zmey::Components::ComponentCompiler G##Name_ComponentManagerRegistration(#Name, ToBlob, FromBlob)

	ZMEY_API const ComponentCompiler& GetComponentCompiler(Hash nameHash);
}

}