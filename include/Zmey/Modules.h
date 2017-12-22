#pragma once

#include <Zmey/Config.h>
#include <Zmey/Job/JobSystem.h>
#include <Zmey/Platform/Platform.h>
#include <Zmey/Graphics/Renderer.h>
#include <Zmey/ResourceLoader/ResourceLoader.h>
#include <Zmey/SettingsManager.h>
#include <Zmey/InputController.h>
#include <Zmey/Physics/PhysicsEngine.h>

namespace Zmey
{
namespace Modules
{
// Global systems
ZMEY_API extern Zmey::Job::IJobSystem* JobSystem;
ZMEY_API extern IPlatform* Platform;
ZMEY_API extern Graphics::RendererInterface* Renderer;
ZMEY_API extern Zmey::ResourceLoader* ResourceLoader;
ZMEY_API extern Zmey::SettingsManager* SettingsManager;
ZMEY_API extern Zmey::InputController* InputController;
ZMEY_API extern Zmey::Physics::PhysicsEngine* PhysicsEngine;

void Initialize();
void ProjectToScripting();
}
}