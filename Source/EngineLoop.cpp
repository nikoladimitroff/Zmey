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

	auto lambda = []()
	{
		FORMAT_LOG(Info, Temp, "from thread %d", std::this_thread::get_id());
	};
	auto id = Modules::ResourceLoader->LoadResource("Content\\Meshes\\Vampire_A_Lusth\\Vampire_A_Lusth.dae");
	while (g_Run)
	{
		Modules::Platform->PumpMessages(windowHandle);

		Modules::TaskSystem->SpawnTask("Log", lambda);
		Modules::TaskSystem->SpawnTask("Log", lambda);
		Modules::TaskSystem->SpawnTask("Log", lambda);
		Modules::TaskSystem->SpawnTask("Log", lambda);

		float clearColor[] = {1.0f, 0.0f, 0.0f, 1.0f};
		Modules::Renderer->ClearBackbufferSurface(clearColor);
		if (auto resource = Modules::ResourceLoader->As<aiScene>(id))
		{
			volatile int x;
			x = 5;
		}
	}

	Modules::Platform->KillWindow(windowHandle);
}

EngineLoop::~EngineLoop()
{}

}