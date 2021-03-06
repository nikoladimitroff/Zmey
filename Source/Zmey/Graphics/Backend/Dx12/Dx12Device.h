#pragma once

#include <Zmey/Config.h>
#ifdef USE_DX12

#include <Zmey/Graphics/Backend/Device.h>
#include <Zmey/Graphics/Backend/Buffer.h>

#include <Zmey/Graphics/Backend/Dx12/Dx12Helpers.h>
#include <Zmey/Memory/MemoryManagement.h>
#include <Zmey/Math/Math.h>

namespace Zmey
{
namespace Graphics
{
namespace Backend
{

class Dx12GraphicsPipelineState : public GraphicsPipelineState
{
public:
	ID3D12PipelineState* PipelineState;
	ID3D12RootSignature* RootSignature;
};

class Dx12Framebuffer : public Framebuffer
{
public:
	ID3D12Resource* TextureResource;
	D3D12_CPU_DESCRIPTOR_HANDLE RTV;
	ID3D12Resource* DSResource;
	D3D12_CPU_DESCRIPTOR_HANDLE DSV;
};

class Dx12ImageView : public ImageView
{
public:
	//VkImageView ImageView;
};

class Dx12Buffer : public Buffer
{
public:
	ID3D12Resource* Buffer;
	D3D12_RESOURCE_STATES State;

	virtual void* Map() override;

	virtual void Unmap() override;
};

class Dx12Device : public Device
{
public:
	Dx12Device();

	virtual void Initialize(WindowHandle windowHandle) override;

	virtual GraphicsPipelineState* CreateGraphicsPipelineState(const GraphicsPipelineStateDesc& desc) override;
	virtual void DestroyGraphicsPipelineState(GraphicsPipelineState* state) override;

	virtual CommandList* CreateCommandList() override;
	virtual void DestroyCommandList(CommandList* list) override;
	virtual void SubmitCommandLists(CommandList** lists, uint32_t count) override;

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

	virtual UVector2 GetSwapChainSize() override { return m_SwapChainSize; }

private:
	ComPtr<IDXGIFactory4> m_Factory;
	ComPtr<ID3D12Device> m_Device;
	ComPtr<ID3D12CommandQueue> m_GraphicsQueue;
	ComPtr<IDXGISwapChain3> m_SwapChain;
	ComPtr<ID3D12DescriptorHeap> m_RTVHeap;
	ComPtr<ID3D12DescriptorHeap> m_DSVHeap;
	ComPtr<ID3D12Resource> m_DSV;
	D3D12_CPU_DESCRIPTOR_HANDLE m_DSVHandle;
	struct SwapChainImage
	{
		ComPtr<ID3D12Resource> Resource;
		D3D12_CPU_DESCRIPTOR_HANDLE RTVHandle;
	};
	stl::vector<SwapChainImage> m_SwapChainImages;
	ComPtr<ID3D12Fence> m_Fence;
	UINT64 m_FenceValue;
	HANDLE m_FenceEvent;

	UVector2 m_SwapChainSize;
};

}
}
}

#endif