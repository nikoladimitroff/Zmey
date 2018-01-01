#pragma once

#include <Zmey/Config.h>
#ifdef USE_VULKAN

#include <Zmey/Graphics/Backend/Buffer.h>

#include <Zmey/Graphics/Backend/Vulkan/VulkanHelpers.h>

namespace Zmey
{
namespace Graphics
{
namespace Backend
{

class VulkanBuffer : public Buffer
{
public:
	VkBuffer UploadBufferHandle;
	VkBuffer BufferHandle;
	VkDeviceMemory UploadMemory;
	VkDeviceMemory Memory;

	class VulkanDevice* Device;

	virtual void* Map() override;

	virtual void Unmap() override;
};

}
}
}

#endif