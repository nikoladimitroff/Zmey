#pragma once
#include <Zmey/Components/ComponentRegistry.h>

#include <algorithm>

#include <Zmey/Scripting/Binding.h>

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
	InstantiateDelegate instantiate,
	DefaultsToBlobDelegate defaultsToBlob,
	ToBlobDelegate toBlob)
	: Name(Zmey::Hash(Zmey::HashHelpers::CaseInsensitiveStringWrapper(nameHash)))
	, Index(ComponentIndex)
	, Instantiate(instantiate)
	, DefaultsToBlob(defaultsToBlob)
	, ToBlob(toBlob)
{
	GComponentRegistry[ComponentIndex] = this;
}

void ExportComponentsToScripting()
{
	ComponentIndex i = 0u;
	stl::small_vector<JsValueRef> componentPrototypes;
	for (const ComponentManagerEntry* entry = GetComponentManagerAtIndex(0); entry; entry = GetComponentManagerAtIndex(++i))
	{
		componentPrototypes.push_back((JsValueRef)entry->ScriptingPrototype);
	}
	Zmey::Chakra::Binding::RegisterPrototypesForAnyTypeSet(Zmey::Hash("Manager"), componentPrototypes);
}

void EmptyDefaultsToBlobImplementation(IDataBlob& blob)
{}
void EmptyToBlobImplementation(const nlohmann::json&, IDataBlob& blob)
{}

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