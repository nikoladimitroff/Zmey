#include <Zmey/Modules.h>

#include "Platform/WindowsPlatform.h" // TODO(alex):include ???

namespace Zmey
{
namespace Modules
{
Zmey::TaskSystem<4>* TaskSystem;
Zmey::IPlatform* Platform;

void Initialize()
{
	TaskSystem = StaticAlloc<Zmey::TaskSystem<4>>();
	Platform = StaticAlloc<Zmey::WindowsPlatform>();
}
}
}