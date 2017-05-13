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

ComponentCompiler::ComponentCompiler(HashType nameHash, ToBlobDelegate toBlob, FromBlobDelegate fromBlob)
	: NameHash(nameHash)
	, ToBlob(toBlob)
	, FromBlob(fromBlob)
{
	ASSERT_FATAL(GCurrentComponentIndex < GComponentRegistry.size());
	GComponentRegistry[GCurrentComponentIndex++] = this;
}

const ComponentCompiler& GetComponentCompiler(uint64_t nameHash)
{
	auto it = std::find_if(GComponentRegistry.begin(), GComponentRegistry.end(), [nameHash](const ComponentCompiler* compiler)
	{
		return compiler->NameHash == nameHash;
	});
	ASSERT_FATAL(it != GComponentRegistry.end());
	return **it;
}
}

}