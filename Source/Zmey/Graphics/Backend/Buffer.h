#pragma once

#include <stdint.h>

namespace Zmey
{
namespace Graphics
{
namespace Backend
{
class Buffer
{
public:
	virtual void* Map() = 0;
	virtual void Unmap() = 0;

	uint32_t Size;
};
}
}
}