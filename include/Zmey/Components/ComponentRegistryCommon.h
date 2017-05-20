#pragma once
namespace Zmey
{
using ComponentIndex = uint16_t;
}
#define DECLARE_COMPONENT_MANAGER() \
	public: \
		const static Zmey::ComponentIndex SZmeyComponentManagerIndex
