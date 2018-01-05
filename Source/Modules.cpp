#include <thread>
#include <Zmey/Modules.h>

#include "Platform/WindowsPlatform.h" // TODO(alex):include ???

namespace Zmey
{
GlobalModules Modules;

// Helper macros to simplify initialization
#define INIT_EMPTY_FIRST_MODULE(ModuleName) \
	: ModuleName(*m_##ModuleName)

#define INIT_EMPTY_MODULE(ModuleName) \
	, ModuleName(*m_##ModuleName)

#define INIT_FIRST_MODULE(ModuleName, ModuleValue) \
	: m_##ModuleName(std::move(ModuleValue)) \
	, ModuleName(*m_##ModuleName)

#define INIT_MODULE(ModuleName, ModuleValue) \
	, m_##ModuleName(std::move(ModuleValue)) \
	, ModuleName(*m_##ModuleName)

// Required because references must be initialized explicitly
GlobalModules::GlobalModules()
	INIT_EMPTY_FIRST_MODULE(JobSystem)
	INIT_EMPTY_MODULE(Platform)
	INIT_EMPTY_MODULE(Renderer)
	INIT_EMPTY_MODULE(ResourceLoader)
	INIT_EMPTY_MODULE(SettingsManager)
	INIT_EMPTY_MODULE(InputController)
	INIT_EMPTY_MODULE(PhysicsEngine)
{
}

GlobalModules::GlobalModules(bool /*initializeFlag*/)
	// TODO: Num fibers and fiber stack size should be configurable
	INIT_FIRST_MODULE(JobSystem, Job::CreateJobSystem(std::thread::hardware_concurrency(), 64, 2 * 1024 * 1024))
	INIT_MODULE(Platform, global::make_unique<Zmey::WindowsPlatform>())
	INIT_MODULE(Renderer, global::make_unique<Zmey::Graphics::Renderer>())
	INIT_MODULE(ResourceLoader, global::make_unique<Zmey::ResourceLoader>())
	INIT_MODULE(SettingsManager, global::make_unique<Zmey::SettingsManager>())
	INIT_MODULE(InputController, global::make_unique<Zmey::InputController>())
	INIT_MODULE(PhysicsEngine, global::make_unique<Zmey::Physics::PhysicsEngine>())
{
}
void GlobalModules::Initialize()
{
	// As hacky as this looks, we need to initialize the references in the instance
	// and that's only possible if we run the ctor another time
	new (this) GlobalModules(true);
}
void GlobalModules::Uninitialize()
{
	this->~GlobalModules();
	new (this) GlobalModules();
}

#undef INIT_EMPTY_FIRST_MODULE
#undef INIT_EMPTY_MODULE
#undef INIT_FIRST_MODULE
#undef INIT_MODULE

}