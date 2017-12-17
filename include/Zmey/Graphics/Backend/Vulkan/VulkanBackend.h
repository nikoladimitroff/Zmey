#pragma once

#include <Zmey/Config.h>
#ifdef USE_VULKAN

#include <Zmey/Graphics/Backend/Backend.h>

#include <Zmey/Graphics/Backend/Vulkan/VulkanHelpers.h>
#include <Zmey/Memory/MemoryManagement.h>

namespace Zmey
{
namespace Graphics
{
namespace Backend
{

//class VulkanSemaphore : public Semaphore
//{
//public:
//	VkSemaphore Semaphore;
//};

class VulkanPipelineState : public PipelineState
{
public:
	VkPipelineLayout Layout;
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

class VulkanBuffer : public Buffer
{
public:
	VkBuffer UploadBufferHandle;
	VkBuffer BufferHandle;
	VkDeviceMemory UploadMemory;
	VkDeviceMemory Memory;

	class VulkanBackend* Backend;

	virtual void* Map() override;

	virtual void Unmap() override;
};


class VulkanBackend : public Backend
{
public:
	VulkanBackend();

	virtual void Initialize(WindowHandle windowHandle) override;

	virtual Shader* CreateShader() override;
	virtual void DestroyShader(Shader* shader) override;
	virtual PipelineState* CreatePipelineState(const PipelineStateDesc& desc) override;
	virtual void DestroyPipelineState(PipelineState* state) override;

	virtual CommandList* CreateCommandList() override;
	virtual void DestroyCommandList(CommandList* list) override;
	virtual void SubmitCommandList(CommandList* list) override;

	//virtual Semaphore* CreateCommandListSemaphore() override;
	//virtual void DestroySemaphore(Semaphore* semaphore) override;

	//virtual RenderPass* CreateRenderPass() override;
	//virtual void DestroyRenderPass(RenderPass* pass) override;

	virtual Framebuffer* CreateFramebuffer(ImageView* imageView) override;
	virtual void DestroyFramebuffer(Framebuffer* framebuffer) override;

	virtual ImageView* CreateImageView() override;
	virtual void DestroyImageView(ImageView* imageView) override;

	virtual Buffer* CreateBuffer(uint32_t size, BufferUsage usage) override;
	virtual void DestroyBuffer(Buffer* buffer) override;

	virtual uint32_t GetSwapChainBuffers() override;
	virtual ImageView* GetSwapChainImageView(uint32_t index) override;

	virtual uint32_t AcquireNextSwapChainImage() override;
	virtual void Present(uint32_t imageIndex) override;

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

	VulkanCommandPoolHandle m_CommandPool{ VulkanCommandPoolDeleter{&m_Device} };

	// Depth stencil
	VkImage m_DepthStencil;
	VkDeviceMemory m_DepthStencilMemory;
	VkImageView m_DepthStencilView;

	VkRenderPass m_Pass;
	VkSemaphore m_ImageAvailable;
	VkSemaphore m_RenderFinishedAvailable;

	friend class VulkanBuffer; // TODO: remove
};

}
}
}

#endif