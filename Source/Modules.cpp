#include <Zmey/Modules.h>

#include "Platform/WindowsPlatform.h" // TODO(alex):include ???
#include "Renderer/VulkanRenderer.h" // TODO(alex):include ???
#include "Scripting/ChakraScriptEngine.h"

namespace Zmey
{
namespace Modules
{
Zmey::TaskSystem<4>* TaskSystem;
Zmey::IPlatform* Platform;
Zmey::IRenderer* Renderer;
Zmey::ResourceLoader* ResourceLoader;
Zmey::IScriptEngine* ScriptEngine;

void Initialize()
{
	TaskSystem = StaticAlloc<Zmey::TaskSystem<4>>();

	Platform = StaticAlloc<Zmey::WindowsPlatform>();
	Renderer = StaticAlloc<Zmey::Renderer::VulkanRenderer>();

	ResourceLoader = StaticAlloc<Zmey::ResourceLoader>();
	ScriptEngine = StaticAlloc<Zmey::Chakra::ChakraScriptEngine>();
}
}
}