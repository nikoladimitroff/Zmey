#pragma once

#include <Zmey/Renderer/Renderer.h>

#include "VulkanHelpers.h"

#include <vector>

namespace Zmey
{
namespace Renderer
{

inline void DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
	if (func != nullptr)
	{
		func(instance, callback, pAllocator);
	}
}

class VulkanRenderer : public IRenderer
{
public:
	virtual bool CreateWindowSurface(WindowHandle handle) override;

	virtual void ClearBackbufferSurface(float color[4]) override;

	virtual void DrawScene() override;

private:
	float m_ClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	VDeleter<VkInstance> m_Instance{ vkDestroyInstance };

	VDeleter<VkDebugReportCallbackEXT> m_Callback{ m_Instance, DestroyDebugReportCallbackEXT };

	VDeleter<VkSurfaceKHR> m_Surface{ m_Instance, vkDestroySurfaceKHR };

	VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;

	VDeleter<VkDevice> m_Device{ vkDestroyDevice };

	VkQueue m_GraphicsQueue;
	VkQueue m_PresentQueue;

	VDeleter<VkSwapchainKHR> m_SwapChain{ m_Device, vkDestroySwapchainKHR };

	// TODO(alex): is std allocator good enough ?
	std::vector<VkImage> m_SwapChainImages;
	std::vector<VDeleter<VkImageView>> m_SwapChainImageViews;
	VkFormat m_SwapChainImageFormat;
	VkExtent2D m_SwapChainExtent;

	VDeleter<VkRenderPass> m_RenderPass{ m_Device, vkDestroyRenderPass };

	VDeleter<VkDescriptorSetLayout> m_DescriptorSetLayout{ m_Device, vkDestroyDescriptorSetLayout };
	VDeleter<VkPipelineLayout> m_PipelineLayout{ m_Device, vkDestroyPipelineLayout };

	VDeleter<VkPipeline> m_GraphicsPipeline{ m_Device, vkDestroyPipeline };

	std::vector<VDeleter<VkFramebuffer>> m_SwapChainFramebuffers;

	VDeleter<VkBuffer> m_VertexBuffer{ m_Device, vkDestroyBuffer };
	VDeleter<VkDeviceMemory> m_VertexBufferMemory{ m_Device, vkFreeMemory };

	VDeleter<VkCommandPool> m_CommandPool{ m_Device, vkDestroyCommandPool };

	VkCommandBuffer m_CommandBuffer;

	VDeleter<VkSemaphore> m_ImageAvailableSemaphore{ m_Device, vkDestroySemaphore };
	VDeleter<VkSemaphore> m_renderFinishedSemaphore{ m_Device, vkDestroySemaphore };

	VDeleter<VkFence> m_LastFrameFinishedFence{ m_Device, vkDestroyFence};
};
}
}