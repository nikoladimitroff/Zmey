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

ComponentManagerEntry::ComponentManagerEntry(const char* nameHash, ComponentIndex ComponentIndex,
	InstantiateDelegate instantiate, DestroyDelegate destroy,
	ToBlobDelegate toBlob, FromBlobDelegate fromBlob)
	: Name(Zmey::Hash(Zmey::HashHelpers::CaseInsensitiveStringWrapper(nameHash)))
	, Instantiate(instantiate)
	, Destroy(destroy)
	, ToBlob(toBlob)
	, FromBlob(fromBlob)
{
	GComponentRegistry[ComponentIndex] = this;
}

const ComponentManagerEntry& GetComponentManager(Hash nameHash)
{
	auto it = std::find_if(GComponentRegistry.begin(), GComponentRegistry.begin() + GCurrentComponentIndex, [nameHash](const ComponentManagerEntry* compiler)
	{
		return compiler->Name == nameHash;
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