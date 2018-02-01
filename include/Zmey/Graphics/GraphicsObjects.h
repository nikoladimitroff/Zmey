#pragma once

#include <stdint.h>

namespace Zmey
{
namespace Graphics
{

using MeshHandle = uint64_t;
using BufferHandle = uint64_t;
using TextureHandle = uint64_t;
using MaterialHandle = uint16_t;

enum class PixelFormat
{
	Unknown,
	B8G8R8A8,
};

}
}