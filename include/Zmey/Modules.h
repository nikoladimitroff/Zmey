#pragma once

#include <Zmey/Tasks/TaskSystem.h>
#include <Zmey/Platform/Platform.h>
#include <Zmey/Graphics/Renderer.h>
#include <Zmey/ResourceLoader/ResourceLoader.h>
#include <Zmey/ScriptEngine.h>

namespace Zmey
{
namespace Modules
{
// Global systems
extern Zmey::TaskSystem<4>* TaskSystem;
extern IPlatform* Platform;
extern Graphics::RendererInterface* Renderer;
extern Zmey::ResourceLoader* ResourceLoader;
extern Zmey::IScriptEngine* ScriptEngine;

void Initialize();
}
}