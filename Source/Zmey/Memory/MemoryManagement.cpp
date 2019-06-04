#include <Zmey/Memory/MemoryManagement.h>
#include <cstddef>

namespace Zmey
{
ZMEY_API Zmey::IAllocator* GAllocator = nullptr;
Zmey::StaticDataAllocator<1024 * 8> GStaticDataAllocator;
template class ThreadLocalLinearAllocator<tls_TempAllocatorSize>;
thread_local Zmey::LinearAllocator<tls_TempAllocatorSize> Zmey::TempAllocator::tls_Alloc;
}

void* operator new(std::size_t size)
{
	return Zmey::GAllocator->Malloc(size, 0);
}
void operator delete(void* ptr)
{
	Zmey::GAllocator->Free(ptr);
}
void* operator new[](std::size_t size)
{
	return Zmey::GAllocator->Malloc(size, 0);
}
void operator delete[](void* ptr)
{
	Zmey::GAllocator->Free(ptr);
}

// Needed for eastl
void* operator new[](size_t size, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{
	return Zmey::GAllocator->Malloc(size, 0);
}

void* operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{
	return Zmey::GAllocator->Malloc(size, static_cast<unsigned>(alignment));
}
