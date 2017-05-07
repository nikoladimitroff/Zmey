#pragma once

#include <Zmey/Tasks/TaskSystem.h>
#include <Zmey/Platform/Platform.h>
#include <Zmey/Renderer/Renderer.h>
#include <Zmey/ResourceLoader/ResourceLoader.h>

namespace Zmey
{
namespace Modules
{
// Global systems
extern Zmey::TaskSystem<4>* TaskSystem;
extern IPlatform* Platform;
extern IRenderer* Renderer;
extern Zmey::ResourceLoader* ResourceLoader;

void Initialize();
}
}