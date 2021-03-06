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

#include <Zmey/Graphics/FrameData.h>

#include <Zmey/Profile.h>

#include <imgui/imgui.h>

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
	Zmey::Modules.Initialize();
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
	Zmey::Modules.JobSystem.RunJobs("Main Scheduler Loop", &runJob, 1, nullptr);
	Zmey::Modules.JobSystem.WaitForCompletion();
	Zmey::Modules.Uninitialize();
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

	Modules.InputController.DispatchActionEventsForFrame(simulateData->DeltaTime);

	Modules.PhysicsEngine.Simulate(simulateData->DeltaTime);
	simulateData->GameInstance->Simulate(simulateData->DeltaTime);
	simulateData->WorldInstance->Simulate(simulateData->DeltaTime);
	Modules.PhysicsEngine.FetchResults();
}

struct GatherDataData
{
	World* WorldInstance;
	Graphics::FrameData& FrameData;
};

void GatherData(void* data)
{
	GatherDataData* gatherData = (GatherDataData*)data;
	Modules.Renderer.GatherData(gatherData->FrameData, *gatherData->WorldInstance);
}

void RenderFrame(void* data)
{
	Graphics::FrameData* frameData = (Graphics::FrameData*)data;
	Modules.Renderer.RenderFrame(*frameData);
}
}

void EngineLoop::RunImpl()
{
	// TODO(alex): get this params from somewhere
	auto width = 1280u;
	auto height = 800u;
	auto windowHandle = Modules.Platform.SpawnWindow(width, height, "Zmey");

	if (!Modules.Renderer.CreateWindowSurface(windowHandle))
	{
		return;
	}

	auto swapchainSizes = Modules.Renderer.GetSwapChainSize();

	uint64_t frameIndex = 0;

	using clock = std::chrono::high_resolution_clock;
	clock::time_point lastFrameTmestamp = clock::now();

	// Create main Player view
	Graphics::View playerView(Graphics::ViewType::PlayerView);
	playerView.SetupProjection(swapchainSizes.x, swapchainSizes.y, glm::radians(90.0f), 0.1f, 1000.0f);
	playerView.SetupView(Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f), Vector3(0.0f, 14.0f, 1.0f));

	Zmey::Name worldName = m_Game->LoadResources();
	Modules.ResourceLoader.WaitForResource(worldName);
	const World* world = Modules.ResourceLoader.AsWorld(worldName);
	ASSERT_FATAL(world);
	Modules.ResourceLoader.ReleaseOwnershipOver(worldName);
	m_World = const_cast<World*>(world);
	m_Game->SetWorld(m_World);

	m_Game->Initialize();
	Modules.PhysicsEngine.SetWorld(*m_World);

	// UI
	{
		auto& io = ImGui::GetIO();
		io.DisplaySize.x = float(swapchainSizes.x);
		io.DisplaySize.y = float(swapchainSizes.y);
		io.IniFilename = nullptr;

		unsigned char* pixels;
		int w, h;
		io.Fonts->GetTexDataAsRGBA32(&pixels, &w, &h);
		io.Fonts->TexID = reinterpret_cast<void*>(Modules.Renderer.UITextureLoaded(pixels, w, h));
		ImGui::StyleColorsDark();
	}

	Job::Counter renderCounter;
	Graphics::FrameData frameDatas[2]; // TODO: 2 seems fine for now
	uint8_t currentFrameData = 0;
	while (g_Run)
	{
		auto frameScope = TempAllocator::GetTlsAllocator().ScopeNow();

		clock::time_point currentFrameTimestamp = clock::now();
		clock::duration timeSinceLastFrame = currentFrameTimestamp - lastFrameTmestamp;
		float deltaTime = timeSinceLastFrame.count() * 1e-9f;

		Modules.Platform.PumpMessages(windowHandle);

		// UI
		{
			auto& io = ImGui::GetIO();
			io.DeltaTime = deltaTime;
			ImGui::NewFrame();
		}

		SimulateData simulateData{ m_Game, m_World, deltaTime };

		Job::Counter simulateCounter;
		Job::JobDecl simulateJob{ SimulateFrame, &simulateData };
		Modules.JobSystem.RunJobs("Simulate", &simulateJob, 1, &simulateCounter);
		Modules.JobSystem.WaitForCounter(&simulateCounter, 0);

		// TODO: Compute visibility
		// Gather render data
		frameDatas[currentFrameData].FrameIndex = frameIndex++;
		playerView.GatherData(frameDatas[currentFrameData]);

		GatherDataData gatherData{ m_World, frameDatas[currentFrameData] };

		Job::Counter gatherDataCounter;
		Job::JobDecl gatherDataJob{ GatherData, &gatherData };
		Modules.JobSystem.RunJobs("GatherData", &gatherDataJob, 1, &gatherDataCounter);
		Modules.JobSystem.WaitForCounter(&gatherDataCounter, 0);

		// Wait for previous Render World job in order to not get ahead more than 1 frame
		Modules.JobSystem.WaitForCounter(&renderCounter, 0);

		Job::JobDecl renderDataJob{ RenderFrame, &frameDatas[currentFrameData] };
		Modules.JobSystem.RunJobs("Render World", &renderDataJob, 1, &renderCounter);
		// There is a no wait here becase we can start next simulate before this has finished

		lastFrameTmestamp = currentFrameTimestamp;

		currentFrameData = (currentFrameData + 1) % 2;
	}
	Modules.JobSystem.WaitForCounter(&renderCounter, 0);

	m_Game->Uninitialize();
	Modules.Platform.KillWindow(windowHandle);
	Modules.JobSystem.Quit();
}

EngineLoop::~EngineLoop()
{
	PROFILE_DESTROY;
}

}