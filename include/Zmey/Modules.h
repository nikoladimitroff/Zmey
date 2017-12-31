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
struct GlobalModules
{
	// Have to provide because of all the refs
	GlobalModules();
	GlobalModules(GlobalModules&&) = delete;
	GlobalModules(const GlobalModules&) = delete;
	GlobalModules& operator=(GlobalModules&&) = delete;
	GlobalModules& operator=(const GlobalModules&) = delete;

	void Initialize();
	void Uninitialize();

private:
	// This constructor creates the actual meaningful object; called internally from GlobalModules::Initialize
	GlobalModules(bool initialize);

	// This layout structure might look a like an overkill but it solves several issues at once:
	// 1. All modules are stored in a statically allocated memory (in global::unique_ptr)
	// 2. There's reference (not a pointer!) to each with a simple name. Semantically, this is correct as these modules should never be null
	// 3. Allows for inter-module dependencies e.g. the InputController depends on the SettingsManager and achieving this effect
	//    requires that the reference to the SettingManager is initialized before the global ptr to InputController
#define DECLARE_MODULE(ModuleClass, ModuleName) \
private: \
	global::unique_ptr<ModuleClass> m_##ModuleName; \
public: \
	ModuleClass& ModuleName

	// Access the module Foo with Zmey::Modules.Foo
	DECLARE_MODULE(Zmey::Job::IJobSystem, JobSystem);
	DECLARE_MODULE(Zmey::IPlatform, Platform);
	DECLARE_MODULE(Zmey::Graphics::RendererInterface, Renderer);
	DECLARE_MODULE(Zmey::ResourceLoader, ResourceLoader);
	DECLARE_MODULE(Zmey::SettingsManager, SettingsManager);
	DECLARE_MODULE(Zmey::InputController, InputController);
	DECLARE_MODULE(Zmey::Physics::PhysicsEngine, PhysicsEngine);

#undef DECLARE_MODULE
};

ZMEY_API extern GlobalModules Modules;
}