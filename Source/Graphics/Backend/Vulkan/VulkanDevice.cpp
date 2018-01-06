#include <Zmey/Graphics/Backend/Vulkan/VulkanDevice.h>

#ifdef USE_VULKAN
#include <Zmey/Logging.h>

#include <Zmey/Graphics/Backend/Vulkan/VulkanShaders.h>

#include <Zmey/Graphics/Backend/Vulkan/VulkanCommandList.h>
#include <Zmey/Graphics/Backend/Vulkan/VulkanBuffer.h>
#include <Zmey/Graphics/Backend/Vulkan/VulkanTexture.h>
#include <Zmey/Graphics/RendererGlobals.h>

namespace Zmey
{
namespace Graphics
{
namespace Backend
{

#ifdef _DEBUG
PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT;
#endif

namespace
{
bool DoesDeviceSupportSwapchain(VkPhysicalDevice device)
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
#ifdef _DEBUG
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
	case VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT:
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

const char* g_Layers[] = {
	"VK_LAYER_LUNARG_standard_validation",
};
#endif

VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const tmp::vector<VkSurfaceFormatKHR>& availableFormats)
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

VkPresentModeKHR ChooseSwapPresentMode(const tmp::vector<VkPresentModeKHR> availablePresentModes)
{
	//for (const auto& mode : availablePresentModes)
	//{
	//	if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
	//	{
	//		return mode;
	//	}
	//}

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
		NOT_REACHED();
		return VkExtent2D{};
	}
}
}

VulkanDevice::VulkanDevice()
{
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
		createInfo.enabledLayerCount = sizeof(g_Layers) / sizeof(const char*);
		createInfo.ppEnabledLayerNames = g_Layers;
#else
		createInfo.enabledLayerCount = 0;
#endif

		if (vkCreateInstance(&createInfo, nullptr, m_Instance.Receive()) != VK_SUCCESS)
		{
			LOG(Fatal, Vulkan, "Failed to create instance");
		}
	}

#ifdef _DEBUG
	{
		Graphics::Backend::vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(m_Instance, "vkDestroyDebugReportCallbackEXT");
		// Add debug report callback
		VkDebugReportCallbackCreateInfoEXT createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
		createInfo.pfnCallback = DebugCallback;

		auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(m_Instance, "vkCreateDebugReportCallbackEXT");
		if (!func)
		{
			LOG(Fatal, Vulkan, "No vkCreateDebugReportCallbackEXT function");
		}

		if (func(m_Instance, &createInfo, nullptr, m_Callback.Receive()) != VK_SUCCESS)
		{
			LOG(Fatal, Vulkan, "Failed to set up vulkan debug callback");
		}
	}
#endif
}


void VulkanDevice::Initialize(WindowHandle windowHandle)
{
	auto scope = TempAllocator::GetTlsAllocator().ScopeNow();

	// Create Window Surface
	{
		VkWin32SurfaceCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		createInfo.hinstance = GetModuleHandle(NULL);
		createInfo.hwnd = HWND(windowHandle);

		if (vkCreateWin32SurfaceKHR(m_Instance, &createInfo, nullptr, m_Surface.Receive()) != VK_SUCCESS)
		{
			LOG(Fatal, Vulkan, "Failed to create win32 surface");
			return;
		}
	}

	// Pick physical device
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);

	if (deviceCount == 0)
	{
		LOG(Fatal, Vulkan, "Failed to find GPU with Vulkan support");
		return;
	}

	tmp::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices.data());

	VkSurfaceCapabilitiesKHR capabilities;
	tmp::vector<VkSurfaceFormatKHR> formats;
	tmp::vector<VkPresentModeKHR> presentModes;

	int graphicsFamilyIndex = -1;
	int presentFamilyIndex = -1;

	for (const auto& device : devices)
	{
		graphicsFamilyIndex = -1;
		presentFamilyIndex = -1;

		if (!DoesDeviceSupportSwapchain(device))
		{
			continue;
		}

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_Surface, &capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, nullptr);

		formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, formats.data());

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presentModeCount, nullptr);

		presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presentModeCount, presentModes.data());

		if (formats.empty() || presentModes.empty())
		{
			continue;
		}

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		tmp::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		for (int i = 0; i < int(queueFamilyCount); ++i)
		{
			if (queueFamilies[i].queueCount > 0 && queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				graphicsFamilyIndex = i;
			}

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_Surface, &presentSupport);

			if (queueFamilies[i].queueCount > 0 && presentSupport)
			{
				presentFamilyIndex = i;
			}

			if (graphicsFamilyIndex != -1 && presentFamilyIndex != -1)
			{
				break;
			}
		}

		if (graphicsFamilyIndex != -1 && presentFamilyIndex != -1)
		{
			m_PhysicalDevice = device;
			break;
		}
	}

	if (m_PhysicalDevice == VK_NULL_HANDLE)
	{
		LOG(Fatal, Vulkan, "Failed to find suitable GPU");
		return;
	}


	// Create logical device
	{
		tmp::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		tmp::vector<int> uniqueQueueFamilies= { graphicsFamilyIndex };
		if (presentFamilyIndex != graphicsFamilyIndex)
		{
			uniqueQueueFamilies.push_back(presentFamilyIndex);
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
			"VK_KHR_maintenance1",
		};

		VkDeviceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.queueCreateInfoCount = uint32_t(queueCreateInfos.size());
		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.ppEnabledExtensionNames = deviceExtensions;
		createInfo.enabledExtensionCount = sizeof(deviceExtensions) / sizeof(const char*);

#ifdef _DEBUG
		createInfo.enabledLayerCount = sizeof(g_Layers) / sizeof(const char*);
		createInfo.ppEnabledLayerNames = g_Layers;
#else
		createInfo.enabledLayerCount = 0;
#endif

		if (vkCreateDevice(m_PhysicalDevice, &createInfo, NULL, m_Device.Receive()) != VK_SUCCESS)
		{
			LOG(Fatal, Vulkan, "Failed to create logical device");
			return;
		}

		vkGetDeviceQueue(m_Device, graphicsFamilyIndex, 0, &m_GraphicsQueue);
		vkGetDeviceQueue(m_Device, presentFamilyIndex, 0, &m_PresentQueue);
	}

	// Create swap chain
	{
		auto surfaceFormat = ChooseSwapSurfaceFormat(formats);
		auto presentMode = ChooseSwapPresentMode(presentModes);
		auto extent = ChooseSwapExtent(capabilities);

		// use at most 2 images, or minImageCount if more than 2
		uint32_t imageCount = capabilities.minImageCount > 2 ? capabilities.minImageCount : 2;
		if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
		{
			imageCount = capabilities.maxImageCount;
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
		uint32_t queueFamilyIndices[] = { uint32_t(graphicsFamilyIndex), uint32_t(presentFamilyIndex) };

		if (graphicsFamilyIndex != presentFamilyIndex)
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

		createInfo.preTransform = capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;

		// TODO(alex): Implement resize
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		if (vkCreateSwapchainKHR(m_Device, &createInfo, nullptr, m_SwapChain.Receive()) != VK_SUCCESS)
		{
			LOG(Fatal, Vulkan, "Failed to create swap chain");
			return;
		}

		vkGetSwapchainImagesKHR(m_Device, m_SwapChain, &imageCount, nullptr);
		m_SwapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(m_Device, m_SwapChain, &imageCount, m_SwapChainImages.data());

		m_SwapChainImageFormat = surfaceFormat.format;
		m_SwapChainExtent = extent;
	}

	// Create image views
	{
		VulkanImageViewDeleter deleter{ &m_Device };
		m_SwapChainImageViews.reserve(m_SwapChainImages.size());
		for (auto i = 0u; i < m_SwapChainImages.size(); ++i)
		{
			m_SwapChainImageViews.emplace_back(deleter);

			VkImageViewCreateInfo viewInfo = {};
			viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewInfo.image = m_SwapChainImages[i];
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewInfo.format = m_SwapChainImageFormat;
			viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = 1;
			viewInfo.subresourceRange.baseArrayLayer = 0;
			viewInfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView(m_Device, &viewInfo, nullptr, m_SwapChainImageViews[i].Receive()) != VK_SUCCESS)
			{
				LOG(Fatal, Vulkan, "failed to create swap chain image view!");
				return;
			}
		}
	}

	// Create depth stencil
	{
		VkImageCreateInfo imageInfo = {};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = m_SwapChainExtent.width;
		imageInfo.extent.height = m_SwapChainExtent.height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = VK_FORMAT_D24_UNORM_S8_UINT;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.flags = 0;

		if (vkCreateImage(m_Device, &imageInfo, nullptr, &m_DepthStencil) != VK_SUCCESS)
		{
			LOG(Fatal, Vulkan, "failed to create depth stencil image!");
			return;
		}

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(m_Device, m_DepthStencil, &memRequirements);

		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &memProperties);

		auto memoryIndex = 0;
		for (auto i = 0u; i < memProperties.memoryTypeCount; ++i)
		{
			if (memRequirements.memoryTypeBits & (1 << i)
				&& (memProperties.memoryTypes[i].propertyFlags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) == VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
			{
				memoryIndex = i;
				break;
			}
		}

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = memoryIndex;

		if (vkAllocateMemory(m_Device, &allocInfo, nullptr, &m_DepthStencilMemory) != VK_SUCCESS)
		{
			LOG(Fatal, Vulkan, "failed to create memory for depth stencil!");
			return;
		}

		vkBindImageMemory(m_Device, m_DepthStencil, m_DepthStencilMemory, 0);

		VkImageViewCreateInfo viewInfo = {};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = m_DepthStencil;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = VK_FORMAT_D24_UNORM_S8_UINT;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(m_Device, &viewInfo, nullptr, &m_DepthStencilView) != VK_SUCCESS)
		{
			LOG(Fatal, Vulkan, "failed to create swap chain image view!");
			return;
		}
	}

	// Initialize other needed stuff
	// Create command pool
	{
		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = graphicsFamilyIndex;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		if (vkCreateCommandPool(m_Device, &poolInfo, nullptr, m_CommandPool.Receive()) != VK_SUCCESS)
		{
			LOG(Fatal, Vulkan, "Failed to create command pool!");
			return;
		}
	}

	// render pass
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = m_SwapChainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription depthAttachment = {};
	depthAttachment.format = VK_FORMAT_D24_UNORM_S8_UINT;
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef = {};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;

	VkAttachmentDescription descriptions[] = { colorAttachment, depthAttachment };
	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 2;
	renderPassInfo.pAttachments = descriptions;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(m_Device, &renderPassInfo, nullptr, &m_Pass) != VK_SUCCESS)
	{
		LOG(Error, Vulkan, "failed to create render pass");
		return;
	}

	{
		VkSemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		if (vkCreateSemaphore(m_Device, &semaphoreInfo, nullptr, &m_ImageAvailable) != VK_SUCCESS)
		{
			LOG(Error, Vulkan, "Failed to create semaphores!");
			return;
		}
	}

	{
		VkSemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		if (vkCreateSemaphore(m_Device, &semaphoreInfo, nullptr, &m_RenderFinishedAvailable) != VK_SUCCESS)
		{
			LOG(Error, Vulkan, "Failed to create semaphores!");
			return;
		}
	}
}

namespace
{
inline VkFormat InputElementFormatToVulkan(InputElementFormat format)
{
	switch (format)
	{
	case InputElementFormat::RGBA8:
		return VK_FORMAT_R8G8B8A8_UNORM;
	case InputElementFormat::Float2:
		return VK_FORMAT_R32G32_SFLOAT;
	case InputElementFormat::Float3:
		return VK_FORMAT_R32G32B32_SFLOAT;
	case InputElementFormat::Float4:
		return VK_FORMAT_R32G32B32A32_SFLOAT;
	default:
		NOT_REACHED();
		break;
	}

	return VK_FORMAT_UNDEFINED;
}

inline VkCullModeFlagBits CullModeToVulkan(CullMode mode)
{
	switch (mode)
	{
	case CullMode::None:
		return VK_CULL_MODE_NONE;
	case CullMode::Front:
		return VK_CULL_MODE_FRONT_BIT;
	case CullMode::Back:
		return VK_CULL_MODE_BACK_BIT;
	default:
		NOT_REACHED();
		return VK_CULL_MODE_NONE;
	}
}
}

GraphicsPipelineState* VulkanDevice::CreateGraphicsPipelineState(const GraphicsPipelineStateDesc& desc)
{
	TEMP_ALLOCATOR_SCOPE;

	VulkanShaderModuleHandle vertShaderModule( VulkanShaderModuleDeleter{&m_Device} );
	VulkanShaderModuleHandle fragShaderModule( VulkanShaderModuleDeleter{&m_Device} );

	{
		VkShaderModuleCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = desc.VertexShader.Size;
		createInfo.pCode = (const uint32_t*)desc.VertexShader.Data;

		if (vkCreateShaderModule(m_Device, &createInfo, nullptr, vertShaderModule.Receive()) != VK_SUCCESS)
		{
			LOG(Error, Vulkan, "failed to create shader module");
			return nullptr;
		}
	}

	{
		VkShaderModuleCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = desc.PixelShader.Size;
		createInfo.pCode = (const uint32_t*)desc.PixelShader.Data;

		if (vkCreateShaderModule(m_Device, &createInfo, nullptr, fragShaderModule.Receive()) != VK_SUCCESS)
		{
			LOG(Error, Vulkan, "failed to create shader module");
			return nullptr;
		}
	}

	VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "VertexShaderMain";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "PixelShaderMain";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	uint32_t totalSize = 0;
	tmp::vector<VkVertexInputAttributeDescription> inputElements;
	inputElements.reserve(desc.Layout.Elements.size());
	uint32_t locationIndex = 0;
	for (auto& ie : desc.Layout.Elements)
	{
		VkVertexInputAttributeDescription ieDesc;
		ieDesc.binding = 0;
		ieDesc.location = locationIndex++;
		ieDesc.format = InputElementFormatToVulkan(ie.Format);
		ieDesc.offset = ie.Offset;
		inputElements.push_back(ieDesc);

		switch (ie.Format)
		{
		case InputElementFormat::RGBA8:
			totalSize += sizeof(uint32_t);
			break;
		case InputElementFormat::Float2:
			totalSize += 2 * sizeof(float);
			break;
		case InputElementFormat::Float3:
			totalSize += 3 * sizeof(float);
			break;
		case InputElementFormat::Float4:
			totalSize += 4 * sizeof(float);
			break;
		default:
			NOT_REACHED();
			break;
		}
	}

	VkVertexInputBindingDescription bindingDescription = {};
	bindingDescription.binding = 0;
	bindingDescription.stride = totalSize;
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.vertexAttributeDescriptionCount = uint32_t(inputElements.size());
	vertexInputInfo.pVertexAttributeDescriptions = inputElements.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; // TODO: take into account topology from desc
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = nullptr;
	viewportState.scissorCount = 1;
	viewportState.pScissors = nullptr;

	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = CullModeToVulkan(desc.Rasterizer.CullMode);
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f;
	rasterizer.depthBiasClamp = 0.0f;
	rasterizer.depthBiasSlopeFactor = 0.0f;

	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f;
	multisampling.pSampleMask = nullptr;
	multisampling.alphaToCoverageEnable = VK_FALSE;
	multisampling.alphaToOneEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = desc.Blend.BlendEnable ? VK_TRUE : VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	VkPushConstantRange ranges[1];
	ranges[0].stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
	ranges[0].size = desc.ResourceTable.NumPushConstants * sizeof(float);
	ranges[0].offset = 0;

	{
		VkSamplerCreateInfo samplerInfo = {};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable = VK_FALSE;
		samplerInfo.maxAnisotropy = 1;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;

		if (vkCreateSampler(m_Device, &samplerInfo, nullptr, &m_StaticSampler) != VK_SUCCESS)
		{
			LOG(Error, Vulkan, "failed to create static sampler");
		}
	}

	VkDescriptorSetLayout setLayout;
	{
		VkDescriptorSetLayoutBinding imageLayoutBinding = {};
		imageLayoutBinding.binding = 0;
		imageLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		imageLayoutBinding.descriptorCount = 1;
		imageLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		imageLayoutBinding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
		samplerLayoutBinding.binding = 1;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
		samplerLayoutBinding.pImmutableSamplers = &m_StaticSampler;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		std::array<VkDescriptorSetLayoutBinding, 2> bindings = { imageLayoutBinding, samplerLayoutBinding };

		VkDescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = uint32_t(bindings.size());
		layoutInfo.pBindings = bindings.data();

		if (vkCreateDescriptorSetLayout(m_Device, &layoutInfo, nullptr, &setLayout) != VK_SUCCESS)
		{
			LOG(Error, Vulkan, "failed to create descriptor set layout");
		}
	}

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &setLayout;
	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges = ranges;

	VkPipelineLayout layout;
	if (vkCreatePipelineLayout(m_Device, &pipelineLayoutInfo, nullptr, &layout) != VK_SUCCESS)
	{
		LOG(Error, Vulkan, "failed to create pipeline layout");
	}

	VkPipelineDepthStencilStateCreateInfo depthStencil = {};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = desc.DepthStencil.DepthEnable ? VK_TRUE : VK_FALSE;
	depthStencil.depthWriteEnable = desc.DepthStencil.DepthWrite ? VK_TRUE : VK_FALSE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.stencilTestEnable = VK_FALSE;

	VkDynamicState dynamicStates[2] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	VkPipelineDynamicStateCreateInfo dynamicStateInfo = {};
	dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicStateInfo.dynamicStateCount = 2;
	dynamicStateInfo.pDynamicStates = dynamicStates;

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicStateInfo;

	pipelineInfo.layout = layout;
	pipelineInfo.renderPass = m_Pass;
	pipelineInfo.subpass = 0;

	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = -1;

	VkPipeline pipeline;
	if (vkCreateGraphicsPipelines(m_Device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
	{
		LOG(Error, Vulkan, "failed to create graphics pipeline!");
	}

	auto result = new VulkanPipelineState;
	result->Layout = layout;
	result->SetLayout = setLayout;
	result->Pipeline = pipeline;
	result->RenderPass = m_Pass;

	return result;
}

void VulkanDevice::DestroyGraphicsPipelineState(GraphicsPipelineState* state)
{
	auto vulkanState = reinterpret_cast<VulkanPipelineState*>(state);
	vkDestroyPipelineLayout(m_Device, vulkanState->Layout, nullptr);
	vkDestroyPipeline(m_Device, vulkanState->Pipeline, nullptr);
	//vkDestroyRenderPass(m_Device, vulkanState->RenderPass, nullptr);
	delete state;
}

uint32_t VulkanDevice::GetSwapChainBuffers()
{
	return uint32_t(m_SwapChainImages.size());
}

ImageView* VulkanDevice::GetSwapChainImageView(uint32_t index)
{
	VkImageView handle = m_SwapChainImageViews[index];
	return reinterpret_cast<ImageView*>(handle);
}

uint32_t VulkanDevice::AcquireNextSwapChainImage()
{
	uint32_t result;
	vkAcquireNextImageKHR(m_Device,
		m_SwapChain,
		std::numeric_limits<uint64_t>::max(),
		m_ImageAvailable,
		VK_NULL_HANDLE,
		&result);

	return result;
}

void VulkanDevice::Present(uint32_t imageIndex)
{
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &m_RenderFinishedAvailable;

	VkSwapchainKHR swapChains[] = { m_SwapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr;

	auto result = vkQueuePresentKHR(m_PresentQueue, &presentInfo);
	if (result != VK_SUCCESS)
	{
		LOG(Error, Vulkan, "Failed to present swap chain image!");
	}

	// TODO: not cool, we should not wait for presentation
	//vkDeviceWaitIdle(m_Device);
}

Framebuffer* VulkanDevice::CreateFramebuffer(ImageView* imageView)
{
	VkImageView attachments[] = {
		reinterpret_cast<VkImageView>(imageView),
		m_DepthStencilView
	};

	VkFramebufferCreateInfo framebufferInfo = {};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = m_Pass;
	framebufferInfo.attachmentCount = 2;
	framebufferInfo.pAttachments = attachments;
	framebufferInfo.width = m_SwapChainExtent.width;
	framebufferInfo.height = m_SwapChainExtent.height;
	framebufferInfo.layers = 1;

	VkFramebuffer fb;
	if (vkCreateFramebuffer(m_Device, &framebufferInfo, nullptr, &fb) != VK_SUCCESS)
	{
		LOG(Fatal, Vulkan, "failed to create framebuffer!");
	}

	auto result = new VulkanFramebuffer;
	result->Framebuffer = fb;
	result->RenderPass = m_Pass;
	result->Width = m_SwapChainExtent.width;
	result->Height = m_SwapChainExtent.height;
	return result;
}

void VulkanDevice::DestroyFramebuffer(Framebuffer* framebuffer)
{
	vkDestroyFramebuffer(m_Device, reinterpret_cast<VulkanFramebuffer*>(framebuffer)->Framebuffer, nullptr);
	delete framebuffer;
}

CommandList* VulkanDevice::CreateCommandList()
{
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = m_CommandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer buffer;
	if (vkAllocateCommandBuffers(m_Device, &allocInfo, &buffer) != VK_SUCCESS)
	{
		LOG(Error, Vulkan, "Cannot allocate command buffer");
		return nullptr;
	}

	VkDescriptorPoolSize poolSize = {};
	poolSize.type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	poolSize.descriptorCount = 1024; // TODO: random number

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = &poolSize;
	poolInfo.maxSets = 1024; // TODO: random

	VkDescriptorPool pool;
	if (vkCreateDescriptorPool(m_Device, &poolInfo, nullptr, &pool) != VK_SUCCESS)
	{
		LOG(Error, Vulkan, "Cannot allocate descriptor pool");
		return nullptr;
	}

	auto result = new VulkanCommandList;
	result->CmdBuffer = buffer;
	result->Pool = pool;

	return result;
}

void VulkanDevice::DestroyCommandList(CommandList* list)
{
	delete list;
}

void VulkanDevice::SubmitCommandLists(CommandList** lists, uint32_t count)
{
	TEMP_ALLOCATOR_SCOPE;
	tmp::vector<VkCommandBuffer> cmdBuffers;
	cmdBuffers.reserve(count);
	for (auto i = 0u; i < count; ++i)
	{
		cmdBuffers.push_back(reinterpret_cast<VulkanCommandList*>(lists[i])->CmdBuffer);
	}

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { m_ImageAvailable };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = count;
	submitInfo.pCommandBuffers = cmdBuffers.data();

	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &m_RenderFinishedAvailable;

	if (vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, nullptr) != VK_SUCCESS)
	{
		LOG(Error, Vulkan, "Failed to submit draw command buffer!");
	}
}

namespace
{
void CreateBufferInternal(VkDevice device, VkPhysicalDevice phDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& outbuffer, VkDeviceMemory& outbufferMemory)
{
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkBuffer buffer;
	if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
	{
		return;
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(phDevice, &memProperties);

	auto memoryIndex = 0;
	for (auto i = 0u; i < memProperties.memoryTypeCount; ++i)
	{
		if (memRequirements.memoryTypeBits & (1 << i)
			&& (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			memoryIndex = i;
			break;
		}
	}

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = memoryIndex;

	VkDeviceMemory bufferMemory;
	if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
	{
		return;
	}

	vkBindBufferMemory(device, buffer, bufferMemory, 0);

	outbuffer = buffer;
	outbufferMemory = bufferMemory;
}
}

Buffer* VulkanDevice::CreateBuffer(uint32_t size, BufferUsage usage)
{
	VkBuffer uploadBuffer;
	VkDeviceMemory uploadMemory;
	CreateBufferInternal(m_Device,
		m_PhysicalDevice,
		size,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		uploadBuffer,
		uploadMemory);

	VkBufferUsageFlags flags = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	switch (usage)
	{
	case BufferUsage::Vertex:
		flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		break;
	case BufferUsage::Index:
		flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		break;
	case BufferUsage::Copy:
		break;
	default:
		NOT_REACHED();
	}

	VkBuffer buffer;
	VkDeviceMemory memory;
	CreateBufferInternal(m_Device,
		m_PhysicalDevice,
		size,
		flags,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		buffer,
		memory);

	auto result = new VulkanBuffer;
	result->UploadBufferHandle = uploadBuffer;
	result->UploadMemory = uploadMemory;
	result->BufferHandle = buffer;
	result->Memory = memory;
	result->Device = this;
	result->Size = size;

	return result;
}

void VulkanDevice::DestroyBuffer(Buffer* buffer)
{
	auto vkBuffer = reinterpret_cast<VulkanBuffer*>(buffer);
	vkDestroyBuffer(m_Device, vkBuffer->BufferHandle, nullptr);
	vkFreeMemory(m_Device, vkBuffer->Memory, nullptr);
	delete buffer;
}

Texture* VulkanDevice::CreateTexture(uint32_t width, uint32_t height, PixelFormat format)
{
	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = PixelFormatToVulkan(format);
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
	imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.flags = 0;

	VkImage image;
	if (vkCreateImage(m_Device, &imageInfo, nullptr, &image) != VK_SUCCESS)
	{
		return nullptr;
	}

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(m_Device, image, &memRequirements);

	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &memProperties);

	auto memoryIndex = 0;
	for (auto i = 0u; i < memProperties.memoryTypeCount; ++i)
	{
		if (memRequirements.memoryTypeBits & (1 << i)
			&& (memProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
		{
			memoryIndex = i;
			break;
		}
	}

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = memoryIndex;

	VkDeviceMemory imageMemory;
	if (vkAllocateMemory(m_Device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
	{
		return nullptr;
	}

	vkBindImageMemory(m_Device, image, imageMemory, 0);

	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = PixelFormatToVulkan(format);
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	VkImageView view;
	if (vkCreateImageView(m_Device, &viewInfo, nullptr, &view) != VK_SUCCESS)
	{
		LOG(Error, Vulkan, "failed to create texture image view!");
	}

	auto result = new VulkanTexture;
	result->TextureHandle = image;
	result->Memory = imageMemory;
	result->Layout = VK_IMAGE_LAYOUT_PREINITIALIZED;
	result->View = view;
	result->Width = width;
	result->Height = height;
	result->Format = format;

	return result;
}

void VulkanDevice::DestroyTexture(Texture* texture)
{
	auto vkBuffer = reinterpret_cast<VulkanTexture*>(texture);
	vkDestroyImage(m_Device, vkBuffer->TextureHandle, nullptr);
	vkFreeMemory(m_Device, vkBuffer->Memory, nullptr);
	delete texture;
}

UVector2 VulkanDevice::GetSwapChainSize()
{
	return UVector2{ m_SwapChainExtent.width, m_SwapChainExtent.height };
}

//TODO: move out to new file
void* VulkanBuffer::Map()
{
	auto device = reinterpret_cast<VulkanDevice*>(Globals::g_Device)->GetNativeDevice();

	void* mappedMemory;
	vkMapMemory(device, UploadMemory, 0, Size, 0, &mappedMemory);
	return mappedMemory;
}

void VulkanBuffer::Unmap()
{
	//TODO: FIX this method :/

	auto device = reinterpret_cast<VulkanDevice*>(Globals::g_Device)->GetNativeDevice();
	vkUnmapMemory(device, UploadMemory);

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = Device->m_CommandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	VkBufferCopy copyRegion = {};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = Size;
	vkCmdCopyBuffer(commandBuffer, UploadBufferHandle, BufferHandle, 1, &copyRegion);

	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(Device->m_GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(Device->m_GraphicsQueue);

	vkFreeCommandBuffers(device, Device->m_CommandPool, 1, &commandBuffer);
}

// TODO(alex): extract to non vulkan header
Device* CreateBackendDevice()
{
	auto result = ZmeyMalloc(sizeof(VulkanDevice));
	new (result) VulkanDevice();

	return reinterpret_cast<Device*>(result);
}

}
}
}
#endif