#pragma once

#include <Zmey/Graphics/Backend/Backend.h>

#include <Zmey/Graphics/Backend/Dx12/Dx12Helpers.h>
#include <Zmey/Memory/MemoryManagement.h>

namespace Zmey
{
namespace Graphics
{
namespace Backend
{

class Dx12PipelineState : public PipelineState
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
};

class Dx12ImageView : public ImageView
{
public:
	//VkImageView ImageView;
};

class Dx12Backend : public Backend
{
public:
	Dx12Backend();

	virtual void Initialize(WindowHandle windowHandle) override;

	virtual Shader* CreateShader() override;
	virtual void DestroyShader(Shader* shader) override;
	virtual PipelineState* CreatePipelineState() override;
	virtual void DestroyPipelineState(PipelineState* state) override;

	virtual CommandList* CreateCommandList() override;
	virtual void DestroyCommandList(CommandList* list) override;
	virtual void SubmitCommandList(CommandList* list) override;

	virtual Framebuffer* CreateFramebuffer(ImageView* imageView) override;
	virtual void DestroyFramebuffer(Framebuffer* framebuffer) override;

	virtual ImageView* CreateImageView() override;
	virtual void DestroyImageView(ImageView* imageView) override;

	virtual uint32_t GetSwapChainBuffers() override;
	virtual ImageView* GetSwapChainImageView(uint32_t index) override;

	virtual uint32_t AcquireNextSwapChainImage() override;
	virtual void Present(uint32_t imageIndex) override;

private:
	ComPtr<IDXGIFactory4> m_Factory;
	ComPtr<ID3D12Device> m_Device;
	ComPtr<ID3D12CommandQueue> m_GraphicsQueue;
	ComPtr<IDXGISwapChain3> m_SwapChain;
	ComPtr<ID3D12DescriptorHeap> m_RTVHeap;
	struct SwapChainImage
	{
		ComPtr<ID3D12Resource> Resource;
		D3D12_CPU_DESCRIPTOR_HANDLE RTVHandle;
	};
	stl::vector<SwapChainImage> m_SwapChainImages;
	ComPtr<ID3D12CommandAllocator> m_CommandAllocator;
	ComPtr<ID3D12Fence> m_Fence;
	UINT64 m_FenceValue;
	HANDLE m_FenceEvent;
};

}
}
}