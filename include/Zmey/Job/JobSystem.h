#pragma once

#include <Zmey/Config.h>

#include <inttypes.h>
#include <atomic>

namespace Zmey
{
namespace Job
{

using JobEntryPoint = void(*)(void*);

struct JobDecl
{
	JobEntryPoint EntryPoint;
	void* Data;
};

// NB: Compile with Enable Fiber-Safe Optimizations

// This is public to allow stack allocation
// Do not modify or set Value. The JobSystem will use it
struct Counter
{
	std::atomic<unsigned> Value;
};

class IJobSystem
{
public:
	// Can be called from anywhere
	virtual void RunJobs(const char* name, JobDecl* jobs, uint32_t numJobs, Counter* counter = nullptr) = 0;

	// Note only 1 job can wait on some counter. This sounds reasonable for now. Implement if ever needed.
	// Can be called only from a Job
	virtual void WaitForCounter(Counter* counter, uint32_t value) = 0;

	// Will set internal flag to quit all fibers after they finish their current task
	// After that worker threads will stop.
	virtual void Quit() = 0;

	// Waits for all threads to finish
	virtual void Destroy() = 0;
};

IJobSystem* CreateJobSystem(uint32_t numWorkerThreads, uint32_t numFibers, uint32_t fiberStackSize);
}
}