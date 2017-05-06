#include "VulkanRenderer.h" // TODO(alex): include ???
#include <Zmey/Logging.h>
#include <Zmey/Memory/MemoryManagement.h> // For TempAllocator

namespace Zmey
{
namespace Renderer
{
namespace
{
VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
	VkDebugReportFlagsEXT flags,
	VkDebugReportObjectTypeEXT objType,
	uint64_t obj,
	size_t location,
	int32_t code,
	const char* layerPrefix,
	const char* msg,
	void* userData)
{
	const char* formatMsg = "Vulkan validation layer: %s\n";

	switch (flags)
	{
	case VK_DEBUG_REPORT_WARNING_BIT_EXT:
		FORMAT_LOG(Warning, formatMsg, msg);
		break;
	case VK_DEBUG_REPORT_ERROR_BIT_EXT:
		FORMAT_LOG(Error, formatMsg, msg);
		break;
	default:
		break;
	}

	return VK_FALSE;
}

// TODO(alex): remove start
struct QueueFamilyIndices
{
	int GraphicsFamily = -1;
	int PresentFamily = -1;

	bool IsComplete()
	{
		return GraphicsFamily >= 0 && PresentFamily >= 0;
	}
};

QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	tmp::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies)
	{
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.GraphicsFamily = i;
		}

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

		if (queueFamily.queueCount > 0 && presentSupport)
		{
			indices.PresentFamily = i;
		}

		if (indices.IsComplete())
		{
			break;
		}

		++i;
	}

	return indices;
}

bool CheckDeviceExtensionSupport(VkPhysicalDevice device)
{
	uint32_t extensionsCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionsCount, nullptr);

	tmp::vector<VkExtensionProperties> availableExtensions(extensionsCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionsCount, availableExtensions.data());

	for (const auto& extension : availableExtensions)
	{
		if (strcmp(extension.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
		{
			return true;
		}
	}

	return false;
}

struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR Capabilities;
	std::vector<VkSurfaceFormatKHR> Formats;
	std::vector<VkPresentModeKHR> PresentMode;
};

SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.Capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

	if (formatCount != 0)
	{
		details.Formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.Formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

	if (formatCount != 0)
	{
		details.PresentMode.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.PresentMode.data());
	}

	return details;
}


bool IsDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	/*VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
	&& deviceFeatures.geometryShader;*/

	QueueFamilyIndices indices = FindQueueFamilies(device, surface);

	bool extensionsSupported = CheckDeviceExtensionSupport(device);

	bool swapChainAdequate = false;
	if (extensionsSupported)
	{
		SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device, surface);
		swapChainAdequate = !swapChainSupport.Formats.empty() && !swapChainSupport.PresentMode.empty();
	}

	return indices.IsComplete() && extensionsSupported && swapChainAdequate;
}

VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED)
	{
		return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	}

	for (const auto& format : availableFormats)
	{
		if (format.format == VK_FORMAT_B8G8R8A8_UNORM
			&& format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return format;
		}
	}

	return availableFormats[0];
}

VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes)
{
	for (const auto& mode : availablePresentModes)
	{
		if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			return mode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilites)
{
	if (capabilites.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return capabilites.currentExtent;
	}
	else
	{
		// TODO(alex): refactor this
		int width = 1280, height = 720;
		VkExtent2D actualExtent = { uint32_t(width), uint32_t(height) };

		actualExtent.width = std::max(capabilites.minImageExtent.width, std::min(capabilites.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilites.minImageExtent.height, std::min(capabilites.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}

// TODO(alex): remove end
}

bool VulkanRenderer::CreateWindowSurface(WindowHandle handle)
{
	auto scope = TempAllocator::GetTlsAllocator().ScopeNow();

#ifdef _DEBUG
	const char* layers[] = {
		"VK_LAYER_LUNARG_standard_validation",
	};
#endif

	// Create Vulkan Instance
	{
		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Zmey Game"; // TODO(alex): get game name
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "Zmey";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		// TODO(alex): decouple from win32
		const char* extensions[] = {
			"VK_KHR_surface",
			"VK_KHR_win32_surface",
	#ifdef _DEBUG
			VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
	#endif
		};

		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledExtensionCount = sizeof(extensions) / sizeof(const char*);
		createInfo.ppEnabledExtensionNames = extensions;

#ifdef _DEBUG
		createInfo.enabledLayerCount = sizeof(layers) / sizeof(const char*);
		createInfo.ppEnabledLayerNames = layers;
#else
		createInfo.enabledLayerCount = 0;
#endif

		if (vkCreateInstance(&createInfo, nullptr, m_Instance.Replace()) != VK_SUCCESS)
		{
			LOG(Fatal, "Vulkan: failed to create instance");
			return false;
		}
	}

#ifdef _DEBUG
	{
		// Add debug report callback
		VkDebugReportCallbackCreateInfoEXT createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
		createInfo.pfnCallback = DebugCallback;

		auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(m_Instance, "vkCreateDebugReportCallbackEXT");
		if (!func)
		{
			LOG(Fatal, "Vulkan: No vkCreateDebugReportCallbackEXT function");
			return false;
		}

		if (func(m_Instance, &createInfo, nullptr, m_Callback.Replace()) != VK_SUCCESS)
		{
			LOG(Fatal, "Vulkan: failed to set up vulkan debug callback");
			return false;
		}
	}
#endif

	// Create Window Surface
	{
		VkWin32SurfaceCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		createInfo.hinstance = GetModuleHandle(NULL);
		createInfo.hwnd = HWND(handle);

		if (vkCreateWin32SurfaceKHR(m_Instance, &createInfo, nullptr, m_Surface.Replace()) != VK_SUCCESS)
		{
			LOG(Fatal, "Vulkan: failed to create win32 surface");
			return false;
		}
	}


	// Pick physical device
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);

		if (deviceCount == 0)
		{
			LOG(Fatal, "Vulkan: failed to find GPU with Vulkan support");
			return false;
		}

		
		tmp::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices.data());

		for (const auto& device : devices)
		{
			if (IsDeviceSuitable(device, m_Surface))
			{
				m_PhysicalDevice = device;
				break;
			}
		}

		if (m_PhysicalDevice == VK_NULL_HANDLE)
		{
			LOG(Fatal, "Vulkan: failed to find suitable GPU");
			return false;
		}
	}

	// Create logical device
	{
		QueueFamilyIndices indices = FindQueueFamilies(m_PhysicalDevice, m_Surface);

		tmp::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		tmp::vector<int> uniqueQueueFamilies = { indices.GraphicsFamily, indices.PresentFamily };
		if (uniqueQueueFamilies[0] == uniqueQueueFamilies[1])
		{
			uniqueQueueFamilies.pop_back();
		}

		float queuePriority = 1.0f;
		for (auto& queueFamily : uniqueQueueFamilies)
		{
			VkDeviceQueueCreateInfo queueCreateInfo = {};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;

			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures = {};

		const char* deviceExtensions[] = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		};

		VkDeviceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.queueCreateInfoCount = uint32_t(queueCreateInfos.size());
		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.ppEnabledExtensionNames = deviceExtensions;
		createInfo.enabledExtensionCount = sizeof(deviceExtensions) / sizeof(const char*);

#ifdef _DEBUG
		createInfo.enabledLayerCount = sizeof(layers) / sizeof(const char*);
		createInfo.ppEnabledLayerNames = layers;
#else
		createInfo.enabledLayerCount = 0;
#endif

		if (vkCreateDevice(m_PhysicalDevice, &createInfo, NULL, m_Device.Replace()) != VK_SUCCESS)
		{
			LOG(Fatal, "Vulkan: failed to create logical device");
			return false;
		}

		vkGetDeviceQueue(m_Device, indices.GraphicsFamily, 0, &m_GraphicsQueue);
		vkGetDeviceQueue(m_Device, indices.PresentFamily, 0, &m_PresentQueue);
	}

	// Create swap chain
	{
		SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(m_PhysicalDevice, m_Surface);

		auto surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.Formats);
		auto presentMode = ChooseSwapPresentMode(swapChainSupport.PresentMode);
		auto extent = ChooseSwapExtent(swapChainSupport.Capabilities);

		uint32_t imageCount = swapChainSupport.Capabilities.minImageCount + 1;
		if (swapChainSupport.Capabilities.maxImageCount > 0 && imageCount > swapChainSupport.Capabilities.maxImageCount)
		{
			imageCount = swapChainSupport.Capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = m_Surface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		auto indices = FindQueueFamilies(m_PhysicalDevice, m_Surface);
		uint32_t queueFamilyIndices[] = { uint32_t(indices.GraphicsFamily), uint32_t(indices.PresentFamily) };

		if (indices.GraphicsFamily != indices.PresentFamily)
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0;
			createInfo.pQueueFamilyIndices = nullptr;
		}

		createInfo.preTransform = swapChainSupport.Capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;

		VkSwapchainKHR oldSwapChain = m_SwapChain;
		createInfo.oldSwapchain = oldSwapChain;

		VkSwapchainKHR newSwapChain;
		if (vkCreateSwapchainKHR(m_Device, &createInfo, nullptr, &newSwapChain) != VK_SUCCESS)
		{
			LOG(Fatal, "Vulkan: failed to create swap chain");
			return false;
		}

		m_SwapChain = newSwapChain;

		vkGetSwapchainImagesKHR(m_Device, m_SwapChain, &imageCount, nullptr);
		m_SwapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(m_Device, m_SwapChain, &imageCount, m_SwapChainImages.data());

		m_SwapChainImageFormat = surfaceFormat.format;
		m_SwapChainExtent = extent;
	}

	// Create command pool
	{
		QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(m_PhysicalDevice, m_Surface);

		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = queueFamilyIndices.GraphicsFamily;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		if (vkCreateCommandPool(m_Device, &poolInfo, nullptr, m_CommandPool.Replace()) != VK_SUCCESS)
		{
			LOG(Fatal, "Vulkan: failed to create command pool!");
			return false;
		}
	}

	// Create command buffers
	{
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = m_CommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		if (vkAllocateCommandBuffers(m_Device, &allocInfo, &m_CommandBuffer) != VK_SUCCESS)
		{
			LOG(Fatal, "Vulkan: Cannot allocate command buffer");
			return false;
		}
	}

	// Create semaphores
	{
		VkSemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		if (vkCreateSemaphore(m_Device, &semaphoreInfo, nullptr, m_ImageAvailableSemaphore.Replace()) != VK_SUCCESS
			|| vkCreateSemaphore(m_Device, &semaphoreInfo, nullptr, m_renderFinishedSemaphore.Replace()) != VK_SUCCESS)
		{
			LOG(Fatal, "Vulkan: failed to create semaphores!");
			return false;
		}
	}

	// Create Fence
	{
		VkFenceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		createInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		if (vkCreateFence(m_Device, &createInfo, nullptr, m_LastFrameFinishedFence.Replace()))
		{
			LOG(Fatal, "Vulkan: failed to create fence");
			return false;
		}
	}

	return true;
}

void VulkanRenderer::ClearBackbufferSurface(float color[4])
{
	vkResetFences(m_Device, 1, &m_LastFrameFinishedFence);

	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(m_Device, m_SwapChain, std::numeric_limits<uint64_t>::max(), m_ImageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

	VkImageSubresourceRange subResourceRange = {};
	subResourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subResourceRange.baseMipLevel = 0;
	subResourceRange.levelCount = 1;
	subResourceRange.baseArrayLayer = 0;
	subResourceRange.layerCount = 1;

	VkImageMemoryBarrier presentToClearBarrier = {};
	presentToClearBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	presentToClearBarrier.srcAccessMask = 0;
	presentToClearBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	presentToClearBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	presentToClearBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	presentToClearBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	presentToClearBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	presentToClearBarrier.image = m_SwapChainImages[imageIndex];
	presentToClearBarrier.subresourceRange = subResourceRange;

	// Change layout of image to be optimal for presenting
	VkImageMemoryBarrier clearToPresentBarrier = {};
	clearToPresentBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	clearToPresentBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	clearToPresentBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	clearToPresentBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	clearToPresentBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	clearToPresentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	clearToPresentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	clearToPresentBarrier.image = m_SwapChainImages[imageIndex];
	clearToPresentBarrier.subresourceRange = subResourceRange;

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	beginInfo.pInheritanceInfo = nullptr;

	// This will implicitly reset the buffer
	vkBeginCommandBuffer(m_CommandBuffer, &beginInfo);

	vkCmdPipelineBarrier(m_CommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &presentToClearBarrier);

	VkClearColorValue colorValue;
	memcpy(colorValue.float32, color, sizeof(color));
	vkCmdClearColorImage(m_CommandBuffer, m_SwapChainImages[imageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &colorValue, 1, &subResourceRange);

	vkCmdPipelineBarrier(m_CommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &clearToPresentBarrier);

	if (vkEndCommandBuffer(m_CommandBuffer) != VK_SUCCESS)
	{
		LOG(Fatal, "Vulkan: failed to record command buffer!");
	}

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { m_ImageAvailableSemaphore };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_CommandBuffer;

	VkSemaphore signalSemaphores[] = { m_renderFinishedSemaphore };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, m_LastFrameFinishedFence) != VK_SUCCESS)
	{
		LOG(Fatal, "Vulkan: failed to submit draw command buffer!");
	}

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { m_SwapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr;

	result = vkQueuePresentKHR(m_PresentQueue, &presentInfo);
	if (result != VK_SUCCESS)
	{
		LOG(Fatal, "Vulkan: failed to present swap chain image!");
	}

	vkWaitForFences(m_Device, 1, &m_LastFrameFinishedFence, true, 160000000);
}
}
}