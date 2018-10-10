#pragma once

namespace Zmey
{

class IAllocator
{
public:
	virtual ~IAllocator() {}
	virtual void* Malloc(size_t size, unsigned alignment) = 0;
	virtual void Free(void* ptr) = 0;
	virtual void* Realloc(void* ptr, size_t newSize) = 0;
};

}