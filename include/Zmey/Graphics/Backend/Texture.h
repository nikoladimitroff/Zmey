#pragma once

#include <stdint.h>
#include <Zmey/Graphics/GraphicsObjects.h>

namespace Zmey
{
namespace Graphics
{
namespace Backend
{

struct Texture
{
	virtual ~Texture() {}

	uint32_t Width;
	uint32_t Height;
	PixelFormat Format;
};

}
}
}