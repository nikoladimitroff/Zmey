#include <Zmey/EngineLoop.h>

#include <chrono>
#include <iostream>

#include <Zmey/Memory/Allocator.h>
#include <Zmey/Memory/MemoryManagement.h>
#include <Zmey/Logging.h>
#include <Zmey/Modules.h>
#include <Zmey/World.h>
#include <Zmey/Components/ComponentRegistry.h>
#include <Zmey/Components/TransformManager.h>
#include <Zmey/Components/RectangleManager.h>
#include <Zmey/Scripting/Binding.h>

#include <Zmey/Graphics/FrameData.h>
#include <Zmey/Graphics/Features.h>

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

static ResourceId magicianClassId;
EngineLoop::EngineLoop(const char* initialWorld)
	: m_World(nullptr)
{
	Zmey::GAllocator = StaticAlloc<MallocAllocator>();
	Zmey::GLogHandler = StaticAlloc<StdOutLogHandler>();
	Zmey::Modules::Initialize();
	m_WorldResourceId = Modules::ResourceLoader->LoadResource(initialWorld);
	magicianClassId = Modules::ResourceLoader->LoadResource("IncineratedDataCache/TestClass.bin");

	Zmey::Components::ExportComponentsToScripting();

	Zmey::Modules::InputController->AddListenerForAction(Zmey::Hash("movecam"), [](float axisValue)
	{
		FORMAT_LOG(Info, Temp, "movecam was called! axisValue: %f", axisValue);
	});
	Zmey::Modules::InputController->AddListenerForAction(Zmey::Hash("jump"), [](float axisValue)
	{
		FORMAT_LOG(Info, Temp, "jump was called! axisValue: %f", axisValue);
	});
	Zmey::Modules::InputController->AddListenerForAction(Zmey::Hash("walk"), [](float axisValue)
	{
		FORMAT_LOG(Info, Temp, "walk was called! axisValue: %f", axisValue);
	});
}
void EngineLoop::Run()
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

	auto meshId = Modules::ResourceLoader->LoadResource("Content\\Meshes\\Vampire_A_Lusth\\Vampire_A_Lusth.dae");

	// Create main Player view
	Graphics::View playerView(Graphics::ViewType::PlayerView);
	playerView.SetupProjection(width, height, glm::radians(60.0f), 0.1f, 1000.0f);
	playerView.SetupView(Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 1.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f));

	auto scriptId = Modules::ResourceLoader->LoadResource("Content\\Scripts\\main.js");
	while (g_Run)
	{
		auto frameScope = TempAllocator::GetTlsAllocator().ScopeNow();

		Modules::Platform->PumpMessages(windowHandle);
		while (frameIndex >= 2 && !Modules::Renderer->CheckIfFrameCompleted(frameIndex - 2))
		{
			// Wait for renderer to catch up
			Modules::Platform->PumpMessages(windowHandle);
		}

		if (!m_World && Modules::ResourceLoader->IsResourceReady(m_WorldResourceId))
		{
			m_World = Modules::ResourceLoader->TakeOwnershipOver<World>(m_WorldResourceId);
			Zmey::Chakra::Binding::ProjectGlobal(L"world", m_World, Zmey::Hash("world"));
		}

		Zmey::Hash magicianHash(Zmey::HashHelpers::CaseInsensitiveStringWrapper("magician"));
		if (magicianClassId != -1 && Modules::ResourceLoader->IsResourceReady(magicianClassId))
		{
			auto vec = Modules::ResourceLoader->As<stl::vector<uint8_t>>(magicianClassId);
			m_World->AddClassToRegistry(magicianHash, vec->data(), vec->size());
			magicianClassId = -1;
		}
		if (magicianClassId == -1)
		{
			m_World->SpawnActor(magicianHash);
		}

		clock::time_point currentFrameTimestamp = clock::now();
		clock::duration timeSinceLastFrame = currentFrameTimestamp - lastFrameTmestamp;
		float deltaTime = timeSinceLastFrame.count() * 1e-9f;
	
		if (Modules::ResourceLoader->IsResourceReady(scriptId))
		{
			Modules::ScriptEngine->ExecuteFromFile(scriptId);
			scriptId = -1;
		}

		Modules::ScriptEngine->ExecuteNextFrame(deltaTime);
		Modules::InputController->DispatchActionEventsForFrame();

		tmp::vector<Graphics::Rect> rectsToDraw;
		if (m_World)
		{
			m_World->Simulate(deltaTime);
			rectsToDraw = m_World->GetManager<Zmey::Components::RectangleManager>().GetRectsToRender();
		}
		// Rendering stuff

		// TODO: Compute visibility

		// Gather render data
		Graphics::FrameData frameData;

		frameData.FrameIndex = frameIndex++;
		playerView.GatherData(frameData);
		if (Modules::ResourceLoader->IsResourceReady(meshId))
		{
			Graphics::Features::MeshRenderer::GatherData(frameData, *Modules::ResourceLoader->As<Graphics::MeshHandle>(meshId));
		}
		Graphics::Features::RectRenderer::GatherData(frameData, rectsToDraw.data(), unsigned(rectsToDraw.size()));

		// TODO: From this point graphics stuff should be on render thread
		Modules::Renderer->RenderFrame(frameData);
		lastFrameTmestamp = currentFrameTimestamp;
	}

	Modules::Renderer->Unitialize();
	Modules::Platform->KillWindow(windowHandle);
}

EngineLoop::~EngineLoop()
{}

}