#pragma once

#include <Zmey/Config.h>
#ifdef USE_VULKAN

#include <Zmey/Graphics/Backend/Texture.h>

#include <Zmey/Graphics/Backend/Vulkan/VulkanHelpers.h>

namespace Zmey
{
namespace Graphics
{
namespace Backend
{

struct VulkanTexture : Texture
{
	VkImage TextureHandle;
	VkDeviceMemory Memory;
	VkImageLayout Layout;
	VkImageView View;
};

}
}
}

#endif