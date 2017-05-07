#include <Zmey/Modules.h>

#include "Platform/WindowsPlatform.h" // TODO(alex):include ???
#include "Renderer/VulkanRenderer.h" // TODO(alex):include ???

namespace Zmey
{
namespace Modules
{
Zmey::TaskSystem<4>* TaskSystem;
Zmey::IPlatform* Platform;
Zmey::IRenderer* Renderer;
Zmey::ResourceLoader* ResourceLoader;

void Initialize()
{
	TaskSystem = StaticAlloc<Zmey::TaskSystem<4>>();

	Platform = StaticAlloc<Zmey::WindowsPlatform>();
	Renderer = StaticAlloc<Zmey::Renderer::VulkanRenderer>();

	ResourceLoader = StaticAlloc<Zmey::ResourceLoader>();
}
}
}