#include <Zmey/EngineLoop.h>

#include <iostream>

#include <Zmey/Memory/Allocator.h>
#include <Zmey/Memory/MemoryManagement.h>
#include <Zmey/Logging.h>
#include <Zmey/Modules.h>
#include <Zmey/World.h>

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

EngineLoop::EngineLoop(const char* initialWorld)
	: m_World(nullptr)
{
	Zmey::GAllocator = StaticAlloc<MallocAllocator>();
	Zmey::GLogHandler = StaticAlloc<StdOutLogHandler>();
	Zmey::Modules::Initialize();
	m_WorldResourceId = Modules::ResourceLoader->LoadResource(initialWorld);
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

	stl::vector<Graphics::Rect> rectsToDraw;

	rectsToDraw.push_back(Graphics::Rect{ 0.0f, 0.0f, 0.1f, 0.1f,{ 1.0f, 0.0f, 0.0f, 1.0f } });
	rectsToDraw.push_back(Graphics::Rect{ 0.5f, 0.5f, 0.2f, 0.1f, {0.0f, 0.0f, 1.0f, 0.5f} });

	while (g_Run)
	{
		Modules::Platform->PumpMessages(windowHandle);
		while (frameIndex >= 2 && !Modules::Renderer->CheckIfFrameCompleted(frameIndex - 2))
		{
			// Wait for renderer to catch up
			Modules::Platform->PumpMessages(windowHandle);
		}

		if (!m_World && Modules::ResourceLoader->IsResourceReady(m_WorldResourceId))
		{
			m_World = Modules::ResourceLoader->TakeOwnershipOver<World>(m_WorldResourceId);
		}

		Modules::ScriptEngine->ExecuteNextFrame(0.f);


		// TODO: remove this
		// Animation code ^^
		rectsToDraw[0].width += 0.0001f;
		if (rectsToDraw[0].width > 0.5f)
			rectsToDraw[0].width = 0.0f;


		// Rendering stuff

		// TODO: Compute visibility

		// Gather render data
		Graphics::FrameData frameData;

		frameData.FrameIndex = frameIndex++;
		Graphics::Features::MeshRenderer::GatherData(frameData);
		Graphics::Features::RectRenderer::GatherData(frameData, rectsToDraw.data(), unsigned(rectsToDraw.size()));

		// TODO: From this point graphics stuff should be on render thread
		Modules::Renderer->RenderFrame(frameData);
	}

	Modules::Platform->KillWindow(windowHandle);
}

EngineLoop::~EngineLoop()
{}

}