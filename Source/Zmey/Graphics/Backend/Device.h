#pragma once

#include <stdint.h>
#include <Zmey/Graphics/Backend/BackendDeclarations.h>
#include <Zmey/Graphics/Backend/GraphicsPipelineState.h>
#include <Zmey/Memory/MemoryManagement.h>
#include <Zmey/Platform/Platform.h>
#include <Zmey/Graphics/GraphicsObjects.h>
#include <Zmey/Math/Math.h>

namespace Zmey
{
namespace Graphics
{
namespace Backend
{

struct Framebuffer
{
	uint32_t Width;
	uint32_t Height;
};

class ImageView
{};

class Device
{
public:
	virtual ~Device() {}

	virtual void Initialize(WindowHandle windowHandle) = 0;

	virtual GraphicsPipelineState* CreateGraphicsPipelineState(const GraphicsPipelineStateDesc& desc) = 0;
	virtual void DestroyGraphicsPipelineState(GraphicsPipelineState* state) = 0;

	virtual CommandList* CreateCommandList() = 0;
	virtual void DestroyCommandList(CommandList* list) = 0;
	virtual void SubmitCommandLists(CommandList** lists, uint32_t count) = 0;

	virtual Framebuffer* CreateFramebuffer(ImageView* imageView) = 0;
	virtual void DestroyFramebuffer(Framebuffer* framebuffer) = 0;

	virtual Buffer* CreateBuffer(uint32_t size, BufferUsage usage) = 0;
	virtual void DestroyBuffer(Buffer* buffer) = 0;

	virtual Texture* CreateTexture(uint32_t width, uint32_t height, PixelFormat format) = 0;
	virtual void DestroyTexture(Texture* texture) = 0;

	virtual uint32_t GetSwapChainBuffers() = 0;
	virtual ImageView* GetSwapChainImageView(uint32_t index) = 0;
	virtual uint32_t AcquireNextSwapChainImage() = 0;

	virtual void Present(uint32_t imageIndex) = 0;

	virtual UVector2 GetSwapChainSize() = 0;
};

Device* CreateBackendDevice();
}
}
}