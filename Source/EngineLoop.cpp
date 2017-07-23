#include <Zmey/EngineLoop.h>

#include <chrono>
#include <iostream>

#include <Zmey/Memory/Allocator.h>
#include <Zmey/Memory/MemoryManagement.h>
#include <Zmey/Logging.h>
#include <Zmey/Modules.h>
#include <Zmey/World.h>
#include <Zmey/Game.h>
#include <Zmey/Components/ComponentRegistry.h>
#include <Zmey/Components/TransformManager.h>
#include <Zmey/Components/RectangleManager.h>
#include <Zmey/Scripting/Binding.h>

#include <Zmey/Graphics/FrameData.h>

#include <Zmey/Profile.h>

namespace Zmey
{

class MallocAllocator : public Zmey::IAllocator
{
public:
	virtual void* Malloc(size_t size, unsigned)
	{
		return std::malloc(size);
	}
	virtual void Free(void* ptr)
	{
		std::free(ptr);
	}
	virtual void* Realloc(void* ptr, size_t newSize)
	{
		return std::realloc(ptr, newSize);
	}
};

class StdOutLogHandler : public Zmey::ILogHandler
{
private:
	const char* StringifySeverity(LogSeverity severity)
	{
		switch (severity)
		{
		case LogSeverity::Debug: return "Debug";
		case LogSeverity::Trace: return "Trace";
		case LogSeverity::Info: return "Info";
		case LogSeverity::Warning: return "Warning";
		case LogSeverity::Error: return "Error";
		case LogSeverity::Fatal: return "Fatal";
		default:
			return "Unknown severity";
		}
	}

public:
	virtual void WriteLog(LogSeverity severity, const char* channel, const char* message)
	{
		printf("%s [%s]: %s\r\n", channel, StringifySeverity(severity), message);
	}
};

EngineLoop::EngineLoop(Game* game)
	: m_World(nullptr)
	, m_Game(game)
{
	Zmey::GAllocator = StaticAlloc<MallocAllocator>();
	Zmey::GLogHandler = StaticAlloc<StdOutLogHandler>();
	Zmey::Modules::Initialize();
	Zmey::Components::ExportComponentsToScripting();
}

void EngineLoop::Run()
{
	PROFILE_INITIALIZE;
	// TODO(alex): get this params from somewhere
	auto width = 1280u;
	auto height = 720u;
	auto windowHandle = Modules::Platform->SpawnWindow(width, height, "Zmey");

	if (!Modules::Renderer->CreateWindowSurface(windowHandle))
	{
		return;
	}

	uint64_t frameIndex = 0;

	using clock = std::chrono::high_resolution_clock;
	clock::time_point lastFrameTmestamp = clock::now();

	// Create main Player view
	Graphics::View playerView(Graphics::ViewType::PlayerView);
	playerView.SetupProjection(width, height, glm::radians(60.0f), 0.1f, 1000.0f);
	playerView.SetupView(Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 1.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f));

	Zmey::Name worldName = m_Game->LoadResources();
	Modules::ResourceLoader->WaitForResource(worldName);
	const World* world = Modules::ResourceLoader->AsWorld(worldName);
	ASSERT_FATAL(world);
	Modules::ResourceLoader->ReleaseOwnershipOver(worldName);
	m_World = const_cast<World*>(world);
	Zmey::Chakra::Binding::ProjectGlobal(L"world", m_World, Zmey::Hash("world"));
	Zmey::Modules::ProjectToScripting();
	m_Game->SetWorld(m_World);

	m_Game->Initialize();
	Modules::PhysicsEngine->SetWorld(*m_World);
	while (g_Run)
	{
		PROFILE_SCOPE;

		auto frameScope = TempAllocator::GetTlsAllocator().ScopeNow();

		clock::time_point currentFrameTimestamp = clock::now();
		clock::duration timeSinceLastFrame = currentFrameTimestamp - lastFrameTmestamp;
		float deltaTime = timeSinceLastFrame.count() * 1e-9f;
		Modules::PhysicsEngine->Simulate(deltaTime);

		Modules::Platform->PumpMessages(windowHandle);
		while (frameIndex >= 2 && !Modules::Renderer->CheckIfFrameCompleted(frameIndex - 2))
		{
			// Wait for renderer to catch up
			Modules::Platform->PumpMessages(windowHandle);
		}


		Modules::ScriptEngine->ExecuteNextFrame(deltaTime);
		Modules::InputController->DispatchActionEventsForFrame();

		m_Game->Simulate(deltaTime);
		m_World->Simulate(deltaTime);
		Modules::PhysicsEngine->FetchResults();

		// Rendering stuff

		// TODO: Compute visibility

		// Gather render data
		Graphics::FrameData frameData;

		frameData.FrameIndex = frameIndex++;
		playerView.GatherData(frameData);
		Modules::Renderer->GatherData(frameData, *m_World);

		// TODO: From this point graphics stuff should be on render thread
		Modules::Renderer->RenderFrame(frameData);
		lastFrameTmestamp = currentFrameTimestamp;

		// Set window title
		char title[100];
		snprintf(title, 100, "Zmey. Delta Time: %.1fms", deltaTime * 1e3f);
		Modules::Platform->SetWindowTitle(windowHandle, title);
	}
	m_Game->Uninitialize();
	Modules::Renderer->Unitialize();
	Modules::Platform->KillWindow(windowHandle);

	PROFILE_DESTROY;
}

EngineLoop::~EngineLoop()
{}

}