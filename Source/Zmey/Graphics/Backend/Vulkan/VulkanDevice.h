#pragma once

#include <Zmey/Config.h>
#ifdef USE_VULKAN

#include <Zmey/Graphics/Backend/Device.h>

#include <Zmey/Graphics/Backend/Vulkan/VulkanHelpers.h>
#include <Zmey/Memory/MemoryManagement.h>

namespace Zmey
{
namespace Graphics
{
namespace Backend
{

class VulkanPipelineState : public GraphicsPipelineState
{
public:
	VkPipelineLayout Layout;
	VkDescriptorSetLayout SetLayout;
	VkPipeline Pipeline;
	VkRenderPass RenderPass;
};

class VulkanFramebuffer : public Framebuffer
{
public:
	VkFramebuffer Framebuffer;
	VkRenderPass RenderPass;
};

class VulkanImageView : public ImageView
{
public:
	VkImageView ImageView;
};

class VulkanDevice : public Device
{
public:
	VulkanDevice();

	virtual void Initialize(WindowHandle windowHandle) override;

	virtual GraphicsPipelineState* CreateGraphicsPipelineState(const GraphicsPipelineStateDesc& desc) override;
	virtual void DestroyGraphicsPipelineState(GraphicsPipelineState* state) override;

	virtual CommandList* CreateCommandList() override;
	virtual void DestroyCommandList(CommandList* list) override;
	virtual void SubmitCommandLists(CommandList** list, uint32_t count) override;

	virtual Framebuffer* CreateFramebuffer(ImageView* imageView) override;
	virtual void DestroyFramebuffer(Framebuffer* framebuffer) override;

	virtual Buffer* CreateBuffer(uint32_t size, BufferUsage usage) override;
	virtual void DestroyBuffer(Buffer* buffer) override;

	virtual Texture* CreateTexture(uint32_t width, uint32_t height, PixelFormat format) override;
	virtual void DestroyTexture(Texture* texture) override;

	virtual uint32_t GetSwapChainBuffers() override;
	virtual ImageView* GetSwapChainImageView(uint32_t index) override;

	virtual uint32_t AcquireNextSwapChainImage() override;
	virtual void Present(uint32_t imageIndex) override;

	virtual UVector2 GetSwapChainSize() override;

	VkDevice GetNativeDevice() { return m_Device; }

private:
	VulkanInstanceHandle m_Instance;
#ifdef _DEBUG
	VulkanDebugReportCallbackEXTHandle m_Callback{ VulkanDebugReportCallbackEXTDeleter{&m_Instance} };
#endif
	VulkanSurfaceKHRHandle m_Surface{ VulkanSurfaceKHRDeleter{&m_Instance} };

	VkPhysicalDevice m_PhysicalDevice;
	VulkanDeviceHandle m_Device;

	VkQueue m_GraphicsQueue;
	VkQueue m_PresentQueue;

	// Swap chain data
	VulkanSwapchainKHRHandle m_SwapChain{ VulkanSwapchainKHRDeleter{&m_Device} };
	stl::vector<VkImage> m_SwapChainImages;
	std::vector<VulkanImageViewHandle> m_SwapChainImageViews;
	VkFormat m_SwapChainImageFormat;
	VkExtent2D m_SwapChainExtent;

	VulkanCommandPoolHandle m_CommandPool{ VulkanCommandPoolDeleter{ &m_Device } };

	// Depth stencil
	VkImage m_DepthStencil;
	VkDeviceMemory m_DepthStencilMemory;
	VkImageView m_DepthStencilView;

	VkRenderPass m_Pass;
	VkSemaphore m_ImageAvailable;
	VkSemaphore m_RenderFinishedAvailable;

	VkSampler m_StaticSampler;

	friend class VulkanBuffer; // TODO: remove me
};

}
}
}

#endif