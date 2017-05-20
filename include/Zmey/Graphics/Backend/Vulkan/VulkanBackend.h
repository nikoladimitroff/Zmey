#pragma once

#include <Zmey/Graphics/Backend/Backend.h>

#include <Zmey/Graphics/Backend/Vulkan/VulkanHelpers.h>
#include <Zmey/Memory/MemoryManagement.h>

namespace Zmey
{
namespace Graphics
{
namespace Backend
{

class VulkanSemaphore : public Semaphore
{
public:
	VkSemaphore Semaphore;
};

class VulkanPipelineState : public PipelineState
{
public:
	VkPipelineLayout Layout;
	VkPipeline Pipeline;
};

class VulkanRenderPass : public RenderPass
{
public:
	VkRenderPass RenderPass;
};

class VulkanFramebuffer : public Framebuffer
{
public:
	VkFramebuffer Framebuffer;
};

class VulkanImageView : public ImageView
{
public:
	VkImageView ImageView;
};

class VulkanBackend : public Backend
{
public:
	VulkanBackend();

	virtual void Initialize(WindowHandle windowHandle) override;

	virtual Shader* CreateShader() override;
	virtual void DestroyShader(Shader* shader) override;
	virtual PipelineState* CreatePipelineState(RenderPass* pass) override;
	virtual void DestroyPipelineState(PipelineState* state) override;

	virtual CommandList* CreateCommandList() override;
	virtual void DestroyCommandList(CommandList* list) override;
	virtual void SubmitCommandList(CommandList* list, Semaphore* waitSemaphore, Semaphore* finishSemaphore) override;

	virtual Semaphore* CreateCommandListSemaphore() override;
	virtual void DestroySemaphore(Semaphore* semaphore) override;

	virtual RenderPass* CreateRenderPass() override;
	virtual void DestroyRenderPass(RenderPass* pass) override;

	virtual Framebuffer* CreateFramebuffer(ImageView* imageView, RenderPass* renderPass) override;
	virtual void DestroyFramebuffer(Framebuffer* framebuffer) override;

	virtual ImageView* CreateImageView() override;
	virtual void DestroyImageView(ImageView* imageView) override;

	virtual uint32_t GetSwapChainBuffers() override;
	virtual ImageView* GetSwapChainImageView(uint32_t index) override;

	virtual uint32_t AcquireNextSwapChainImage(Semaphore* waitSem) override;
	virtual void Present(Semaphore* finishSem, uint32_t imageIndex) override;

private:
	VulkanInstanceHandle m_Instance;
#ifdef _DEBUG
	VulkanDebugReportCallbackEXTHandle m_Callback{ VulkanDebugReportCallbackEXTDeleter{m_Instance} };
#endif
	VulkanSurfaceKHRHandle m_Surface{ VulkanSurfaceKHRDeleter{m_Instance} };

	VkPhysicalDevice m_PhysicalDevice;
	VulkanDeviceHandle m_Device;

	VkQueue m_GraphicsQueue;
	VkQueue m_PresentQueue;

	// Swap chain data
	VulkanSwapchainKHRHandle m_SwapChain{ VulkanSwapchainKHRDeleter{m_Device} };
	stl::vector<VkImage> m_SwapChainImages;
	std::vector<VulkanImageViewHandle> m_SwapChainImageViews;
	VkFormat m_SwapChainImageFormat;
	VkExtent2D m_SwapChainExtent;

	VulkanCommandPoolHandle m_CommandPool{ VulkanCommandPoolDeleter{m_Device} };
};

}
}
}