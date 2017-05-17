#pragma once
#include <Zmey/Components/ComponentRegistry.h>

#include <algorithm>

namespace Zmey
{

namespace Components
{
namespace
{
uint16_t GCurrentComponentIndex = 0u;
stl::array<ComponentCompiler*, 256u> GComponentRegistry;
}

ComponentCompiler::ComponentCompiler(const char* nameHash, ToBlobDelegate toBlob, FromBlobDelegate fromBlob)
	: Name(Zmey::Hash(Zmey::HashHelpers::CaseInsensitiveStringWrapper(nameHash)))
	, ToBlob(toBlob)
	, FromBlob(fromBlob)
{
	ASSERT_FATAL(GCurrentComponentIndex < GComponentRegistry.size());
	GComponentRegistry[GCurrentComponentIndex++] = this;
}

const ComponentCompiler& GetComponentCompiler(Hash nameHash)
{
	auto it = std::find_if(GComponentRegistry.begin(), GComponentRegistry.begin() + GCurrentComponentIndex, [nameHash](const ComponentCompiler* compiler)
	{
		return compiler->Name == nameHash;
	});
	ASSERT_FATAL(it != GComponentRegistry.end());
	return **it;
}
}

}