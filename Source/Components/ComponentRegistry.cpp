#pragma once
#include <Zmey/Components/ComponentRegistry.h>

#include <algorithm>

namespace Zmey
{

namespace Components
{
namespace
{
ComponentIndex GCurrentComponentIndex = 0u;
stl::array<ComponentManagerEntry*, 256u> GComponentRegistry;
}

uint16_t GetNextComponentManagerIndex()
{
	ASSERT_FATAL(GCurrentComponentIndex < GComponentRegistry.size());
	return GCurrentComponentIndex++;
}

ComponentManagerEntry::ComponentManagerEntry(const char* fullName, const char* shortName,
	ComponentIndex ComponentIndex,
	InstantiateDelegate instantiate,
	DefaultsToBlobDelegate defaultsToBlob,
	ToBlobDelegate toBlob,
	int8_t priority)
	: FullName(fullName)
	, ShortName(shortName)
	, ShortNameHash(Zmey::Hash(Zmey::HashHelpers::CaseInsensitiveStringWrapper(shortName)))
	, Index(ComponentIndex)
	, Instantiate(instantiate)
	, DefaultsToBlob(defaultsToBlob)
	, ToBlob(toBlob)
	, Priority(priority)
{
	GComponentRegistry[ComponentIndex] = this;
}

void EmptyDefaultsToBlobImplementation(IDataBlob& blob)
{}
void EmptyToBlobImplementation(const nlohmann::json&, IDataBlob& blob)
{}

const ComponentManagerEntry& GetComponentManager(Hash nameHash)
{
	auto it = std::find_if(GComponentRegistry.begin(), GComponentRegistry.begin() + GCurrentComponentIndex, [nameHash](const ComponentManagerEntry* compiler)
	{
		return compiler->ShortNameHash == nameHash;
	});
	ASSERT_FATAL(it != GComponentRegistry.end());
	return **it;
}

const ComponentManagerEntry* GetComponentManagerAtIndex(ComponentIndex index)
{
	if (index >= GComponentRegistry.size())
	{
		return nullptr;
	}
	return GComponentRegistry[index];
}

}

}