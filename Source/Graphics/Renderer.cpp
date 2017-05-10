#include "RendererData.h" // TODO(alex): include ???
#include <Zmey/Logging.h>
#include <Zmey/Memory/MemoryManagement.h> // For TempAllocator
#include <Zmey/Graphics/FrameData.h>
#include <Zmey/Graphics/Renderer.h>

#include <Zmey/Graphics/Features.h>

#include <fstream>

namespace Zmey
{
namespace Graphics
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

struct Vertex
{
	Vector2 pos;
};

static std::vector<char> ReadFile(const std::string& filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		LOG(Fatal, Vulkan, "failed to open file");
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);
	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}

// TODO(alex): remove end
}

bool RendererInterface::CreateWindowSurface(WindowHandle handle)
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

		if (vkCreateInstance(&createInfo, nullptr, m_Data->m_Instance.Replace()) != VK_SUCCESS)
		{
			LOG(Fatal, Vulkan, "Failed to create instance");
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

		auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(m_Data->m_Instance, "vkCreateDebugReportCallbackEXT");
		if (!func)
		{
			LOG(Fatal, Vulkan, "No vkCreateDebugReportCallbackEXT function");
			return false;
		}

		if (func(m_Data->m_Instance, &createInfo, nullptr, m_Data->m_Callback.Replace()) != VK_SUCCESS)
		{
			LOG(Fatal, Vulkan, "Failed to set up vulkan debug callback");
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

		if (vkCreateWin32SurfaceKHR(m_Data->m_Instance, &createInfo, nullptr, m_Data->m_Surface.Replace()) != VK_SUCCESS)
		{
			LOG(Fatal, Vulkan, "Failed to create win32 surface");
			return false;
		}
	}


	// Pick physical device
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(m_Data->m_Instance, &deviceCount, nullptr);

		if (deviceCount == 0)
		{
			LOG(Fatal, Vulkan, "Failed to find GPU with Vulkan support");
			return false;
		}

		
		tmp::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(m_Data->m_Instance, &deviceCount, devices.data());

		for (const auto& device : devices)
		{
			if (IsDeviceSuitable(device, m_Data->m_Surface))
			{
				m_Data->m_PhysicalDevice = device;
				break;
			}
		}

		if (m_Data->m_PhysicalDevice == VK_NULL_HANDLE)
		{
			LOG(Fatal, Vulkan, "Failed to find suitable GPU");
			return false;
		}
	}

	// Create logical device
	{
		QueueFamilyIndices indices = FindQueueFamilies(m_Data->m_PhysicalDevice, m_Data->m_Surface);

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

		if (vkCreateDevice(m_Data->m_PhysicalDevice, &createInfo, NULL, m_Data->m_Device.Replace()) != VK_SUCCESS)
		{
			LOG(Fatal, Vulkan, "Failed to create logical device");
			return false;
		}

		vkGetDeviceQueue(m_Data->m_Device, indices.GraphicsFamily, 0, &m_Data->m_GraphicsQueue);
		vkGetDeviceQueue(m_Data->m_Device, indices.PresentFamily, 0, &m_Data->m_PresentQueue);
	}

	// Create swap chain
	{
		SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(m_Data->m_PhysicalDevice, m_Data->m_Surface);

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
		createInfo.surface = m_Data->m_Surface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		auto indices = FindQueueFamilies(m_Data->m_PhysicalDevice, m_Data->m_Surface);
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

		VkSwapchainKHR oldSwapChain = m_Data->m_SwapChain;
		createInfo.oldSwapchain = oldSwapChain;

		VkSwapchainKHR newSwapChain;
		if (vkCreateSwapchainKHR(m_Data->m_Device, &createInfo, nullptr, &newSwapChain) != VK_SUCCESS)
		{
			LOG(Fatal, Vulkan, "Failed to create swap chain");
			return false;
		}

		m_Data->m_SwapChain = newSwapChain;

		vkGetSwapchainImagesKHR(m_Data->m_Device, m_Data->m_SwapChain, &imageCount, nullptr);
		m_Data->m_SwapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(m_Data->m_Device, m_Data->m_SwapChain, &imageCount, m_Data->m_SwapChainImages.data());

		m_Data->m_SwapChainImageFormat = surfaceFormat.format;
		m_Data->m_SwapChainExtent = extent;
	}

	// Create image views
	{
		m_Data->m_SwapChainImageViews.resize(m_Data->m_SwapChainImages.size(), VDeleter<VkImageView>{ m_Data->m_Device, vkDestroyImageView });
		for (auto i = 0u; i < m_Data->m_SwapChainImages.size(); ++i)
		{
			VkImageViewCreateInfo viewInfo = {};
			viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewInfo.image = m_Data->m_SwapChainImages[i];
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewInfo.format = m_Data->m_SwapChainImageFormat;
			viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = 1;
			viewInfo.subresourceRange.baseArrayLayer = 0;
			viewInfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView(m_Data->m_Device, &viewInfo, nullptr, m_Data->m_SwapChainImageViews[i].Replace()) != VK_SUCCESS)
			{
				LOG(Fatal, Vulkan, "failed to create texture image view!");
				return false;
			}
		}
	}

	// Create render pass
	{
		VkAttachmentDescription colorAttachment = {};
		colorAttachment.format = m_Data->m_SwapChainImageFormat;
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

		if (vkCreateRenderPass(m_Data->m_Device, &renderPassInfo, nullptr, m_Data->m_RenderPass.Replace()) != VK_SUCCESS)
		{
			LOG(Fatal, Vulkan, "failed to create render pass");
			return false;
		}
	}

	// Create descriptor set layout
	{
		VkDescriptorSetLayoutBinding uboLayoutBinding = {};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		uboLayoutBinding.pImmutableSamplers = nullptr;

		std::array<VkDescriptorSetLayoutBinding, 1> bindings = { uboLayoutBinding };

		VkDescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = uint32_t(bindings.size());
		layoutInfo.pBindings = bindings.data();

		if (vkCreateDescriptorSetLayout(m_Data->m_Device, &layoutInfo, nullptr, m_Data->m_DescriptorSetLayout.Replace()) != VK_SUCCESS)
		{
			LOG(Fatal, Vulkan, "failed to crete descriptor set layout!");
			return false;
		}
	}

	// Create graphics pipeline
	{
		auto vertShaderCode = ReadFile("vert.spv");
		auto fragShaderCode = ReadFile("frag.spv");

		VDeleter<VkShaderModule> vertShaderModule{ m_Data->m_Device, vkDestroyShaderModule };
		VDeleter<VkShaderModule> fragShaderModule{ m_Data->m_Device, vkDestroyShaderModule };

		{
			VkShaderModuleCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			createInfo.codeSize = vertShaderCode.size();
			createInfo.pCode = (uint32_t*)vertShaderCode.data();

			if (vkCreateShaderModule(m_Data->m_Device, &createInfo, nullptr, vertShaderModule.Replace()) != VK_SUCCESS)
			{
				LOG(Fatal, Vulkan, "failed to create shader module");
				return false;
			}
		}

		{
			VkShaderModuleCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			createInfo.codeSize = fragShaderCode.size();
			createInfo.pCode = (uint32_t*)fragShaderCode.data();

			if (vkCreateShaderModule(m_Data->m_Device, &createInfo, nullptr, fragShaderModule.Replace()) != VK_SUCCESS)
			{
				LOG(Fatal, Vulkan, "failed to create shader module");
				return false;
			}
		}

		VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		std::array<VkVertexInputAttributeDescription, 1> attributeDescription = {};

		attributeDescription[0].binding = 0;
		attributeDescription[0].location = 0;
		attributeDescription[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescription[0].offset = offsetof(Vertex, pos);

		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.vertexAttributeDescriptionCount = uint32_t(attributeDescription.size());
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescription.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)m_Data->m_SwapChainExtent.width;
		viewport.height = (float)m_Data->m_SwapChainExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.offset = { 0, 0 };
		scissor.extent = m_Data->m_SwapChainExtent;

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
		colorBlendAttachment.blendEnable = VK_FALSE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
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

		VkDescriptorSetLayout setLayout[] = { m_Data->m_DescriptorSetLayout };
		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr; // TODO:
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = 0;

		if (vkCreatePipelineLayout(m_Data->m_Device, &pipelineLayoutInfo, nullptr, m_Data->m_PipelineLayout.Replace()) != VK_SUCCESS)
		{
			LOG(Fatal, Vulkan, "failed to create pipeline layout");
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

		pipelineInfo.layout = m_Data->m_PipelineLayout;
		pipelineInfo.renderPass = m_Data->m_RenderPass;
		pipelineInfo.subpass = 0;

		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineInfo.basePipelineIndex = -1;

		if (vkCreateGraphicsPipelines(m_Data->m_Device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, m_Data->m_GraphicsPipeline.Replace()) != VK_SUCCESS)
		{
			LOG(Fatal, Vulkan, "failed to create graphics pipeline!");
		}
	}

	// Create framebuffers
	{
		m_Data->m_SwapChainFramebuffers.resize(m_Data->m_SwapChainImageViews.size(), VDeleter<VkFramebuffer>{ m_Data->m_Device, vkDestroyFramebuffer });
		for (auto i = 0u; i < m_Data->m_SwapChainFramebuffers.size(); ++i)
		{
			VkImageView attachments[] = {
				m_Data->m_SwapChainImageViews[i]
			};

			VkFramebufferCreateInfo framebufferInfo = {};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = m_Data->m_RenderPass;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width = m_Data->m_SwapChainExtent.width;
			framebufferInfo.height = m_Data->m_SwapChainExtent.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(m_Data->m_Device, &framebufferInfo, nullptr, m_Data->m_SwapChainFramebuffers[i].Replace()) != VK_SUCCESS)
			{
				LOG(Fatal, Vulkan, "failed to create framebuffer!");
			}
		}
	}

	// Create command pool
	{
		QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(m_Data->m_PhysicalDevice, m_Data->m_Surface);

		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = queueFamilyIndices.GraphicsFamily;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		if (vkCreateCommandPool(m_Data->m_Device, &poolInfo, nullptr, m_Data->m_CommandPool.Replace()) != VK_SUCCESS)
		{
			LOG(Fatal, Vulkan, "Failed to create command pool!");
			return false;
		}
	}

	// Create Vertex Buffer
	{
		const std::vector<Vertex> gVertices = {
			{ { -0.5f, -0.5f } },
			{ { 0.5f, -0.5f } },
			{ { 0.5f, 0.5f } },
			{ { -0.5f, 0.5f } }
		};


		auto FindMemoryType = [this](uint32_t typeFilter, VkMemoryPropertyFlags properties)
		{
			VkPhysicalDeviceMemoryProperties memProperties;
			vkGetPhysicalDeviceMemoryProperties(m_Data->m_PhysicalDevice, &memProperties);

			for (auto i = 0u; i < memProperties.memoryTypeCount; ++i)
			{
				if (typeFilter & (1 << i)
					&& (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
				{
					return i;
				}
			}

			LOG(Fatal, Vulkan, "failed to find suitable memory type!");
			return 0u;
		};

		auto CreateBuffer = [this, FindMemoryType](VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VDeleter<VkBuffer>& buffer, VDeleter<VkDeviceMemory>& bufferMemory)
		{
			VkBufferCreateInfo bufferInfo = {};
			bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferInfo.size = size;
			bufferInfo.usage = usage;
			bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			if (vkCreateBuffer(m_Data->m_Device, &bufferInfo, nullptr, buffer.Replace()) != VK_SUCCESS)
			{
				LOG(Fatal, Vulkan, "failed to create buffer!");
			}

			VkMemoryRequirements memRequirements;
			vkGetBufferMemoryRequirements(m_Data->m_Device, buffer, &memRequirements);

			VkMemoryAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

			if (vkAllocateMemory(m_Data->m_Device, &allocInfo, nullptr, bufferMemory.Replace()) != VK_SUCCESS)
			{
				LOG(Fatal, Vulkan, "failed to allocate buffer memory!");
			}

			vkBindBufferMemory(m_Data->m_Device, buffer, bufferMemory, 0);
		};

		auto CopyBuffer = [this](VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
		{
			VkCommandBufferAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandPool = m_Data->m_CommandPool;
			allocInfo.commandBufferCount = 1;

			VkCommandBuffer commandBuffer;
			vkAllocateCommandBuffers(m_Data->m_Device, &allocInfo, &commandBuffer);

			VkCommandBufferBeginInfo beginInfo = {};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

			vkBeginCommandBuffer(commandBuffer, &beginInfo);

			VkBufferCopy copyRegion = {};
			copyRegion.srcOffset = 0;
			copyRegion.dstOffset = 0;
			copyRegion.size = size;
			vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

			vkEndCommandBuffer(commandBuffer);

			VkSubmitInfo submitInfo = {};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &commandBuffer;

			vkQueueSubmit(m_Data->m_GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
			vkQueueWaitIdle(m_Data->m_GraphicsQueue);

			vkFreeCommandBuffers(m_Data->m_Device, m_Data->m_CommandPool, 1, &commandBuffer);
		};

		VkDeviceSize bufferSize = sizeof(gVertices[0]) * gVertices.size();

		VDeleter<VkBuffer> stagingBuffer{ m_Data->m_Device, vkDestroyBuffer };
		VDeleter<VkDeviceMemory> stagingBufferMemory{ m_Data->m_Device, vkFreeMemory };
		CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(m_Data->m_Device, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, gVertices.data(), (size_t)bufferSize);
		vkUnmapMemory(m_Data->m_Device, stagingBufferMemory);

		CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_Data->m_VertexBuffer, m_Data->m_VertexBufferMemory);

		CopyBuffer(stagingBuffer, m_Data->m_VertexBuffer, bufferSize);
	}

	// Create command buffers
	{
		m_Data->m_CommandBuffers.resize(m_Data->m_SwapChainFramebuffers.size());

		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = m_Data->m_CommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = uint32_t(m_Data->m_CommandBuffers.size());

		if (vkAllocateCommandBuffers(m_Data->m_Device, &allocInfo, m_Data->m_CommandBuffers.data()) != VK_SUCCESS)
		{
			LOG(Fatal, Vulkan, "Cannot allocate command buffer");
			return false;
		}
	}

	// Create semaphores
	{
		VkSemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		if (vkCreateSemaphore(m_Data->m_Device, &semaphoreInfo, nullptr, m_Data->m_ImageAvailableSemaphore.Replace()) != VK_SUCCESS
			|| vkCreateSemaphore(m_Data->m_Device, &semaphoreInfo, nullptr, m_Data->m_renderFinishedSemaphore.Replace()) != VK_SUCCESS)
		{
			LOG(Fatal, Vulkan, "Failed to create semaphores!");
			return false;
		}
	}

	// Create Fence
	{
		VkFenceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		createInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		if (vkCreateFence(m_Data->m_Device, &createInfo, nullptr, m_Data->m_LastFrameFinishedFence.Replace()))
		{
			LOG(Fatal, Vulkan, "Failed to create fence");
			return false;
		}
	}

	return true;
}

namespace
{
void PrepareData(FrameData& frameData, RendererData& data)
{
	// TODO: Prepare graphics data
	Features::MeshRenderer::PrepareData(frameData);
}

void GenerateCommands(FrameData& frameData, RendererData& data, uint32_t imageIndex)
{
	// TODO: High level render passes

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	beginInfo.pInheritanceInfo = nullptr;

	// This will implicitly reset the buffer
	vkBeginCommandBuffer(data.m_CommandBuffers[imageIndex], &beginInfo);

	// Begin Main pass on PlayerView
	VkRenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = data.m_RenderPass;
	renderPassInfo.framebuffer = data.m_SwapChainFramebuffers[imageIndex];
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = data.m_SwapChainExtent;

	VkClearValue clearColor = { data.m_ClearColor[0], data.m_ClearColor[1], data.m_ClearColor[2], data.m_ClearColor[3] };
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;

	vkCmdBeginRenderPass(data.m_CommandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	Features::MeshRenderer::GenerateCommands(frameData,
		RenderPass::Main,
		ViewType::PlayerView,
		reinterpret_cast<void*>(data.m_CommandBuffers[imageIndex]),
		data);

	vkCmdEndRenderPass(data.m_CommandBuffers[imageIndex]);
	// End Main pass on PlayerView

	if (vkEndCommandBuffer(data.m_CommandBuffers[imageIndex]) != VK_SUCCESS)
	{
		LOG(Fatal, Vulkan, "Failed to record command buffer!");
	}

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { data.m_ImageAvailableSemaphore };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &data.m_CommandBuffers[imageIndex];

	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &data.m_renderFinishedSemaphore;

	if (vkQueueSubmit(data.m_GraphicsQueue, 1, &submitInfo, nullptr) != VK_SUCCESS)
	{
		LOG(Fatal, Vulkan, "Failed to submit draw command buffer!");
	}
}

void Present(FrameData& frameData, RendererData& data, uint32_t imageIndex)
{
	// TODO: Present logic
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &data.m_renderFinishedSemaphore;

	VkSwapchainKHR swapChains[] = { data.m_SwapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr;

	auto result = vkQueuePresentKHR(data.m_PresentQueue, &presentInfo);
	if (result != VK_SUCCESS)
	{
		LOG(Fatal, Vulkan, "Failed to present swap chain image!");
	}

	// TODO: not cool, we should not wait for presentation
	vkDeviceWaitIdle(data.m_Device);
}
}

void RendererInterface::RenderFrame(FrameData& frameData)
{
	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(m_Data->m_Device, m_Data->m_SwapChain, std::numeric_limits<uint64_t>::max(), m_Data->m_ImageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

	PrepareData(frameData, *m_Data);

	GenerateCommands(frameData, *m_Data, imageIndex);

	Present(frameData, *m_Data, imageIndex);

	m_Data->LastCompletedFrame = frameData.FrameIndex;
}

bool RendererInterface::CheckIfFrameCompleted(uint64_t frameIndex)
{
	return frameIndex <= m_Data->LastCompletedFrame;
}

RendererInterface::RendererInterface()
// TODO: remove this allocation
	: m_Data(new RendererData)
{}
}
}