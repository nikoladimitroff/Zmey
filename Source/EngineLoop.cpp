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
	virtual void WriteLog(LogSeverity severity, const char* message)
	{
		printf("Zmey [%s]: %s\r\n", StringifySeverity(severity), message);
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
		FORMAT_LOG(Info, "from thread %d", std::this_thread::get_id());
	};
	while (g_Run)
	{
		Modules::Platform->PumpMessages(windowHandle);

		Modules::TaskSystem->SpawnTask("Log", lambda);
		Modules::TaskSystem->SpawnTask("Log", lambda);
		Modules::TaskSystem->SpawnTask("Log", lambda);
		Modules::TaskSystem->SpawnTask("Log", lambda);

		float clearColor[] = {1.0f, 0.0f, 0.0f, 1.0f};
		Modules::Renderer->ClearBackbufferSurface(clearColor);
	}

	Modules::Platform->KillWindow(windowHandle);
}

EngineLoop::~EngineLoop()
{}

}