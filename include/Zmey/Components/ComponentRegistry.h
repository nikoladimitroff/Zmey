#pragma once
#include <Zmey/EntityManager.h>
#include <Zmey/Memory/MemoryManagement.h>
#include <Zmey/Hash.h>

class json;
namespace Zmey
{

namespace Components
{
	class IDataBlob
	{
	public:
		virtual ~IDataBlob() {}
		virtual void WriteData(const char* dataName, const uint8_t* data, uint16_t dataSize) = 0;
	};
	struct ComponentCompiler
	{
		using ToBlobDelegate = void (*)(json&, IDataBlob& blob);
		using FromBlobDelegate = void (*)(const tmp::vector<EntityId>& entities, const char* blob);

		ComponentCompiler(HashType nameHash, ToBlobDelegate toBlob, FromBlobDelegate fromBlob);
		const uint64_t NameHash;
		const ToBlobDelegate ToBlob;
		const FromBlobDelegate FromBlob;
	};

#define REGISTER_COMPONENT_MANAGER(Name, ToBlob, FromBlob) \
	static Zmey::Components::ComponentCompiler G##Name_ComponentManagerRegistration(Zmey::Hash(#Name), ToBlob, FromBlob)

	const ComponentCompiler& GetComponentCompiler(uint64_t nameHash);

}

}