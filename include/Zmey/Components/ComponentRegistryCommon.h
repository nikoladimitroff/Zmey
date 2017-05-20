#pragma once
namespace Zmey
{
using ComponentIndex = uint16_t;
}
#define DECLARE_COMPONENT_MANAGER(ClassName) \
	public: \
		##ClassName(World& world) : ComponentManager(world) {} \
		const static Zmey::ComponentIndex SZmeyComponentManagerIndex
