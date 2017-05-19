#pragma once
namespace Zmey
{
namespace Components
{
struct ComponentManagerEntry;
}
}
#define DECLARE_COMPONENT_MANAGER() \
	private: \
		const static uint32_t SZmeyComponentManagerIndex; \
		friend class Zmey::Components::ComponentManagerEntry;
