#include <Zmey/Graphics/Backend/Vulkan/VulkanBackend.h>
#include <Zmey/Logging.h>

#include <Zmey/Graphics/Backend/Vulkan/VulkanShaders.h>

#include <Zmey/Graphics/Backend/Vulkan/VulkanCommandList.h>

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
}

VulkanBackend::VulkanBackend()
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
		createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
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


void VulkanBackend::Initialize(WindowHandle windowHandle)
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

		uint32_t imageCount = capabilities.minImageCount + 1;
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
		VulkanImageViewDeleter deleter{ m_Device };
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
}

Semaphore* VulkanBackend::CreateCommandListSemaphore()
{
	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkSemaphore sem;
	if (vkCreateSemaphore(m_Device, &semaphoreInfo, nullptr, &sem) != VK_SUCCESS)
	{
		LOG(Error, Vulkan, "Failed to create semaphores!");
		return nullptr;
	}

	auto result = new VulkanSemaphore;
	result->Semaphore = sem;
	return result;
}

void VulkanBackend::DestroySemaphore(Semaphore* semaphore)
{
	vkDestroySemaphore(m_Device, reinterpret_cast<VulkanSemaphore*>(semaphore)->Semaphore, nullptr);
	delete semaphore;
}

Shader* VulkanBackend::CreateShader()
{
	return nullptr;
}

void VulkanBackend::DestroyShader(Shader* shader)
{

}

RenderPass* VulkanBackend::CreateRenderPass()
{
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

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
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

	VkRenderPass pass;
	if (vkCreateRenderPass(m_Device, &renderPassInfo, nullptr, &pass) != VK_SUCCESS)
	{
		LOG(Error, Vulkan, "failed to create render pass");
		return nullptr;
	}

	auto result = new VulkanRenderPass;
	result->RenderPass = pass;
	return result;
}

void VulkanBackend::DestroyRenderPass(RenderPass* pass)
{
	auto vulkanPass = reinterpret_cast<VulkanRenderPass*>(pass);
	vkDestroyRenderPass(m_Device, vulkanPass->RenderPass, nullptr);
	delete pass;
}

PipelineState* VulkanBackend::CreatePipelineState(RenderPass* pass)
{
	VulkanShaderModuleHandle vertShaderModule( VulkanShaderModuleDeleter{m_Device} );
	VulkanShaderModuleHandle fragShaderModule( VulkanShaderModuleDeleter{m_Device} );

	{
		VkShaderModuleCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = Shaders::g_RectsVSSize;
		createInfo.pCode = Shaders::g_RectsVS;

		if (vkCreateShaderModule(m_Device, &createInfo, nullptr, vertShaderModule.Receive()) != VK_SUCCESS)
		{
			LOG(Error, Vulkan, "failed to create shader module");
			return nullptr;
		}
	}

	{
		VkShaderModuleCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = Shaders::g_RectsPSSize;
		createInfo.pCode = Shaders::g_RectsPS;

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

	VkVertexInputBindingDescription bindingDescription = {};
	bindingDescription.binding = 0;
	bindingDescription.stride = 0;
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 0;
	vertexInputInfo.pVertexBindingDescriptions = nullptr;
	vertexInputInfo.vertexAttributeDescriptionCount = 0;
	vertexInputInfo.pVertexAttributeDescriptions = nullptr;

	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)m_SwapChainExtent.width;
	viewport.height = (float)m_SwapChainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = m_SwapChainExtent;

	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_NONE;
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
	colorBlendAttachment.blendEnable = VK_TRUE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
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
	ranges[0].size = 8 * sizeof(float);
	ranges[0].offset = 0;
	/*ranges[1].stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
	ranges[1].size = 4 * sizeof(float);
	ranges[1].offset = ranges[0].size;*/

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0;
	pipelineLayoutInfo.pSetLayouts = nullptr; // TODO:
	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges = ranges;

	VkPipelineLayout layout;
	if (vkCreatePipelineLayout(m_Device, &pipelineLayoutInfo, nullptr, &layout) != VK_SUCCESS)
	{
		LOG(Error, Vulkan, "failed to create pipeline layout");
	}

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = nullptr;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = nullptr;

	pipelineInfo.layout = layout;
	pipelineInfo.renderPass = reinterpret_cast<VulkanRenderPass*>(pass)->RenderPass;
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
	result->Pipeline = pipeline;

	return result;
}

void VulkanBackend::DestroyPipelineState(PipelineState* state)
{
	auto vulkanState = reinterpret_cast<VulkanPipelineState*>(state);
	vkDestroyPipelineLayout(m_Device, vulkanState->Layout, nullptr);
	vkDestroyPipeline(m_Device, vulkanState->Pipeline, nullptr);
	delete state;
}

ImageView* VulkanBackend::CreateImageView()
{
	return nullptr;
}

void VulkanBackend::DestroyImageView(ImageView* imageView)
{

}

uint32_t VulkanBackend::GetSwapChainBuffers()
{
	return uint32_t(m_SwapChainImages.size());
}

ImageView* VulkanBackend::GetSwapChainImageView(uint32_t index)
{
	VkImageView handle = m_SwapChainImageViews[index];
	return reinterpret_cast<ImageView*>(handle);
}

uint32_t VulkanBackend::AcquireNextSwapChainImage(Semaphore* waitSem)
{
	uint32_t result;
	vkAcquireNextImageKHR(m_Device,
		m_SwapChain,
		std::numeric_limits<uint64_t>::max(),
		reinterpret_cast<VulkanSemaphore*>(waitSem)->Semaphore,
		VK_NULL_HANDLE,
		&result);

	return result;
}

void VulkanBackend::Present(Semaphore* finishSem, uint32_t imageIndex)
{
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &reinterpret_cast<VulkanSemaphore*>(finishSem)->Semaphore;

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
	vkDeviceWaitIdle(m_Device);
}

Framebuffer* VulkanBackend::CreateFramebuffer(ImageView* imageView, RenderPass* renderPass)
{
	VkImageView attachments[] = {
		reinterpret_cast<VkImageView>(imageView)
	};

	VkFramebufferCreateInfo framebufferInfo = {};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = reinterpret_cast<VulkanRenderPass*>(renderPass)->RenderPass;
	framebufferInfo.attachmentCount = 1; 
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
	return result;
}

void VulkanBackend::DestroyFramebuffer(Framebuffer* framebuffer)
{
	vkDestroyFramebuffer(m_Device, reinterpret_cast<VulkanFramebuffer*>(framebuffer)->Framebuffer, nullptr);
	delete framebuffer;
}

CommandList* VulkanBackend::CreateCommandList()
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

	auto result = new VulkanCommandList;
	result->CmdBuffer = buffer;

	return result;
}

void VulkanBackend::DestroyCommandList(CommandList* list)
{
	delete list;
}

void VulkanBackend::SubmitCommandList(CommandList* list, Semaphore* waitSemaphore, Semaphore* finishSemaphore)
{
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { reinterpret_cast<VulkanSemaphore*>(waitSemaphore)->Semaphore };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &reinterpret_cast<VulkanCommandList*>(list)->CmdBuffer;

	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &reinterpret_cast<VulkanSemaphore*>(finishSemaphore)->Semaphore;

	if (vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, nullptr) != VK_SUCCESS)
	{
		LOG(Error, Vulkan, "Failed to submit draw command buffer!");
	}
}

// TODO(alex): extract to non vulkan header
Backend* CreateBackend()
{
	auto result = ZmeyMalloc(sizeof(VulkanBackend));
	new (result) VulkanBackend();

	return reinterpret_cast<Backend*>(result);
}

}
}
}