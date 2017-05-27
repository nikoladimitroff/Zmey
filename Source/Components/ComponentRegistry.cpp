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

ComponentManagerEntry::ComponentManagerEntry(const char* fullName, const char* shortName,
	ComponentIndex ComponentIndex,
	InstantiateDelegate instantiate,
	DefaultsToBlobDelegate defaultsToBlob,
	ToBlobDelegate toBlob)
	: FullName(fullName)
	, ShortName(shortName)
	, ShortNameHash(Zmey::Hash(Zmey::HashHelpers::CaseInsensitiveStringWrapper(shortName)))
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
	stl::small_vector<const char*> componentNames;
	for (const ComponentManagerEntry* entry = GetComponentManagerAtIndex(0); entry; entry = GetComponentManagerAtIndex(++i))
	{
		Zmey::Hash fullNameHash(Zmey::HashHelpers::CaseInsensitiveStringWrapper(entry->FullName));
		JsValueRef scriptingPrototype = Zmey::Chakra::Binding::AutoNativeClassProjecter::GetPrototypeOf(fullNameHash);
		componentPrototypes.push_back(scriptingPrototype);
		componentNames.push_back(entry->ShortName);
	}
	Zmey::Chakra::Binding::AnyTypeData data("Manager", componentNames, componentPrototypes);
	Zmey::Chakra::Binding::RegisterPrototypesForAnyTypeSet(data);
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