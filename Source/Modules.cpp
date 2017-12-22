#include <thread>
#include <Zmey/Modules.h>

#include "Platform/WindowsPlatform.h" // TODO(alex):include ???

namespace Zmey
{
namespace Modules
{
ZMEY_API Zmey::Job::IJobSystem* JobSystem;
ZMEY_API Zmey::IPlatform* Platform;
ZMEY_API Zmey::Graphics::RendererInterface* Renderer;
ZMEY_API Zmey::ResourceLoader* ResourceLoader;
ZMEY_API Zmey::SettingsManager* SettingsManager;
ZMEY_API Zmey::InputController* InputController;
ZMEY_API Zmey::Physics::PhysicsEngine* PhysicsEngine;

void Initialize()
{
	// TODO: Num fibers and fiber stack size should be configurable
	JobSystem = Job::CreateJobSystem(std::thread::hardware_concurrency(), 64, 2 * 1024 * 1024);

	Platform = StaticAlloc<Zmey::WindowsPlatform>();
	Renderer = StaticAlloc<Zmey::Graphics::RendererInterface>();

	ResourceLoader = StaticAlloc<Zmey::ResourceLoader>();
	SettingsManager = StaticAlloc<Zmey::SettingsManager>();
	InputController = StaticAlloc<Zmey::InputController>();
	PhysicsEngine = StaticAlloc<Zmey::Physics::PhysicsEngine>();
}

}
}