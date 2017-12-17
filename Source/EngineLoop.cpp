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
	PROFILE_INITIALIZE;
}

void EngineLoop::RunJobEntryPoint(void* data)
{
	auto loop = reinterpret_cast<EngineLoop*>(data);
	loop->RunImpl();
}

void EngineLoop::Run()
{
	Job::JobDecl runJob{ RunJobEntryPoint, this };
	Zmey::Modules::JobSystem->RunJobs("Main Scheduler Loop", &runJob, 1, nullptr);
	// TODO: This is needed so we wait for all jobs to finish. Change to something better API wise
	Zmey::Modules::JobSystem->Destroy();
	profiler::dumpBlocksToFile("test_profile.prof");
}

namespace
{
struct SimulateData
{
	Game* GameInstance;
	World* WorldInstance;
	float DeltaTime;
};

void SimulateFrame(void* data)
{
	auto simulateData = (SimulateData*)data;
	Modules::PhysicsEngine->Simulate(simulateData->DeltaTime);

	// TODO(alex): disabled scripting for now
	//Modules::ScriptEngine->ExecuteNextFrame(deltaTime);
	Modules::InputController->DispatchActionEventsForFrame();

	simulateData->GameInstance->Simulate(simulateData->DeltaTime);
	simulateData->WorldInstance->Simulate(simulateData->DeltaTime);
	Modules::PhysicsEngine->FetchResults();
}

struct GatherDataData
{
	World* WorldInstance;
	Graphics::FrameData& FrameData;
};

void GatherData(void* data)
{
	GatherDataData* gatherData = (GatherDataData*)data;
	Modules::Renderer->GatherData(gatherData->FrameData, *gatherData->WorldInstance);
}

void RenderFrame(void* data)
{
	Graphics::FrameData* frameData = (Graphics::FrameData*)data;
	Modules::Renderer->RenderFrame(*frameData);
}

}

void EngineLoop::RunImpl()
{
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
	playerView.SetupView(Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f), Vector3(0.0f, 8.0f, 10.0f));

	Zmey::Name worldName = m_Game->LoadResources();
	Modules::ResourceLoader->WaitForResource(worldName);
	const World* world = Modules::ResourceLoader->AsWorld(worldName);
	ASSERT_FATAL(world);
	Modules::ResourceLoader->ReleaseOwnershipOver(worldName);
	m_World = const_cast<World*>(world);
	//Zmey::Chakra::Binding::ProjectGlobal(L"world", m_World, Zmey::Hash("world"));
	//Zmey::Modules::ProjectToScripting();
	m_Game->SetWorld(m_World);

	m_Game->Initialize();
	Modules::PhysicsEngine->SetWorld(*m_World);

	Job::Counter renderCounter;
	Graphics::FrameData frameDatas[2]; // TODO: 2 seems fine for now
	uint8_t currentFrameData = 0;
	while (g_Run)
	{
		auto frameScope = TempAllocator::GetTlsAllocator().ScopeNow();

		clock::time_point currentFrameTimestamp = clock::now();
		clock::duration timeSinceLastFrame = currentFrameTimestamp - lastFrameTmestamp;
		float deltaTime = timeSinceLastFrame.count() * 1e-9f;
		//FORMAT_LOG(Error, Zmey, "time %f", deltaTime);

		Modules::Platform->PumpMessages(windowHandle);

		SimulateData simulateData{ m_Game, m_World, deltaTime };

		Job::Counter simulateCounter;
		Job::JobDecl simulateJob{ SimulateFrame, &simulateData };
		Modules::JobSystem->RunJobs("Simulate", &simulateJob, 1, &simulateCounter);
		Modules::JobSystem->WaitForCounter(&simulateCounter, 0);

		// TODO: Compute visibility

		// Gather render data
		frameDatas[currentFrameData].FrameIndex = frameIndex++;
		playerView.GatherData(frameDatas[currentFrameData]);

		GatherDataData gatherData{ m_World, frameDatas[currentFrameData] };

		Job::Counter gatherDataCounter;
		Job::JobDecl gatherDataJob{ GatherData, &gatherData };
		Modules::JobSystem->RunJobs("GatherData", &gatherDataJob, 1, &gatherDataCounter);
		Modules::JobSystem->WaitForCounter(&gatherDataCounter, 0);

		// Wait for previous Render World job in order to not get ahead more than 1 frame
		Modules::JobSystem->WaitForCounter(&renderCounter, 0);

		Job::JobDecl renderDataJob{ RenderFrame, &frameDatas[currentFrameData] };
		Modules::JobSystem->RunJobs("Render World", &renderDataJob, 1, &renderCounter);
		// There is a no wait here becase we can start next simulate before this has finished

		lastFrameTmestamp = currentFrameTimestamp;

		currentFrameData = (currentFrameData + 1) % 2;

		// TODO: add some text drawing and draw it onto the screen
		// We cannot use SetWindowTitle because we are changing threads all the time
		// because of our job system and SetWindowTitle can be called only on the thread
		// that created the window or it will hang
		//char title[100];
		//snprintf(title, 100, "Zmey. Delta Time: %.1fms", deltaTime * 1e3f);
		//Modules::Platform->SetWindowTitle(windowHandle, title);
	}
	Modules::JobSystem->WaitForCounter(&renderCounter, 0);

	m_Game->Uninitialize();
	Modules::Renderer->Unitialize();
	Modules::Platform->KillWindow(windowHandle);
	Modules::JobSystem->Quit();
}

EngineLoop::~EngineLoop()
{
	PROFILE_DESTROY;
}

}