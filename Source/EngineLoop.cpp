#include <Zmey/EngineLoop.h>

#include <iostream>

#include <Zmey/Memory/Allocator.h>
#include <Zmey/Memory/MemoryManagement.h>
#include <Zmey/Logging.h>
#include <Zmey/Tasks/TaskSystem.h>

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
}
void EngineLoop::Run()
{
	TaskSystem<4> taskSystem;
	auto lambda = []()
	{
		FORMAT_LOG(Info, "from thread %d", std::this_thread::get_id());
	};
	while (1)
	{
		taskSystem.SpawnTask("Log", lambda);
		taskSystem.SpawnTask("Log", lambda);
		taskSystem.SpawnTask("Log", lambda);
		taskSystem.SpawnTask("Log", lambda);
	}
}
EngineLoop::~EngineLoop()
{}

}