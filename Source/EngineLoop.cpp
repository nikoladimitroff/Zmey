#include <Zmey/EngineLoop.h>

#include <iostream>

#include <Zmey/Memory/Allocator.h>
#include <Zmey/Memory/MemoryManagement.h>
#include <Zmey/Logging.h>
#include <Zmey/Modules.h>

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

EngineLoop::EngineLoop()
{
	Zmey::GAllocator = StaticAlloc<MallocAllocator>();
	Zmey::GLogHandler = StaticAlloc<StdOutLogHandler>();
	Zmey::Modules::Initialize();
}
void EngineLoop::Run()
{
	// TODO(alex): get this params from somewhere
	auto windowHandle = Modules::Platform->SpawnWindow(1280, 720, "Zmey");

	if (!Modules::Renderer->CreateWindowSurface(windowHandle))
	{
		return;
	}

	auto id = Modules::ResourceLoader->LoadResource("Content\\Meshes\\Vampire_A_Lusth\\Vampire_A_Lusth.dae");
	auto scriptId = Modules::ResourceLoader->LoadResource("Content\\Scripts\\main.js");
	while (g_Run)
	{
		Modules::Platform->PumpMessages(windowHandle);

		float clearColor[] = {1.0f, 0.0f, 0.0f, 1.0f};
		Modules::Renderer->ClearBackbufferSurface(clearColor);
		Modules::Renderer->DrawScene();
		if (Modules::ResourceLoader->IsResourceReady(id))
		{
			volatile int x;
			x = 5;
		}
		if (Modules::ResourceLoader->IsResourceReady(scriptId))
		{
			Modules::ScriptEngine->ExecuteFromFile(scriptId);
			scriptId = -1;
		}
		Modules::ScriptEngine->ExecuteNextFrame(0.f);
	}

	Modules::Platform->KillWindow(windowHandle);
}

EngineLoop::~EngineLoop()
{}

}