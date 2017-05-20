#pragma once

#include <stdint.h>
#include <Zmey/Graphics/Backend/BackendDeclarations.h>

#include <Zmey/Platform/Platform.h>

namespace Zmey
{
namespace Graphics
{
namespace Backend
{

class Semaphore
{};

class PipelineState
{};

class RenderPass
{};

class Framebuffer
{};

class ImageView
{};

class Backend
{
public:
	virtual ~Backend() {}

	virtual void Initialize(WindowHandle windowHandle) = 0;

	virtual Shader* CreateShader() = 0;
	virtual void DestroyShader(Shader* shader) = 0;

	virtual PipelineState* CreatePipelineState(RenderPass* pass) = 0;
	virtual void DestroyPipelineState(PipelineState* state) = 0;

	virtual CommandList* CreateCommandList() = 0;
	virtual void DestroyCommandList(CommandList* list) = 0;
	virtual void SubmitCommandList(CommandList* list, Semaphore* waitSemaphore, Semaphore* finishSemaphore) = 0;

	virtual Semaphore* CreateCommandListSemaphore() = 0;
	virtual void DestroySemaphore(Semaphore* semaphore) = 0;

	virtual RenderPass* CreateRenderPass() = 0;
	virtual void DestroyRenderPass(RenderPass* pass) = 0;

	virtual Framebuffer* CreateFramebuffer(ImageView* imageView, RenderPass* renderPass) = 0;
	virtual void DestroyFramebuffer(Framebuffer* framebuffer) = 0;

	virtual ImageView* CreateImageView() = 0;
	virtual void DestroyImageView(ImageView* imageView) = 0;

	virtual uint32_t GetSwapChainBuffers() = 0;
	virtual ImageView* GetSwapChainImageView(uint32_t index) = 0;
	virtual uint32_t AcquireNextSwapChainImage(Semaphore* waitSem) = 0;

	virtual void Present(Semaphore* finishSem, uint32_t imageIndex) = 0;
};

Backend* CreateBackend();
}
}
}