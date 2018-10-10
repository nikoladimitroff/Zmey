#pragma once
#include <Zmey/Config.h>
namespace Zmey
{
using ComponentIndex = uint16_t;
}
#define DECLARE_COMPONENT_MANAGER(ClassName) \
	public: \
		##ClassName(World& world) : ComponentManager(world) {} \
		ZMEY_API const static Zmey::ComponentIndex SZmeyComponentManagerIndex

#define DECLARE_EXTERNAL_COMPONENT_MANAGER(ClassName) \
	public: \
		##ClassName( Zmey::World& world) : ComponentManager(world) {} \
		const static Zmey::ComponentIndex SZmeyComponentManagerIndex
