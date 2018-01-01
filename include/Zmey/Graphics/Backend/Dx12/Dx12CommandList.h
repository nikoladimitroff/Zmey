#pragma once

#include <Zmey/Config.h>
#ifdef USE_DX12

#include <stdint.h>
#include <Zmey/Graphics/Backend/CommandList.h>

#include <Zmey/Graphics/Backend/Dx12/Dx12Helpers.h>

namespace Zmey
{
namespace Graphics
{
namespace Backend
{

class Dx12CommandList : public CommandList
{
public:
	virtual void BeginRecording() override;
	virtual void EndRecording() override;

	virtual void BeginRenderPass(Framebuffer* fb) override;
	virtual void EndRenderPass(Framebuffer* fb) override;

	virtual void BindGraphicsPipelineState(GraphicsPipelineState* state) override;
	virtual void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t startVertex, uint32_t startInstance) override;
	virtual void SetPushConstants(GraphicsPipelineState* layout, uint32_t offset, uint32_t count, const void* data) override;
	virtual void SetShaderResourceView(GraphicsPipelineState* layout, Texture* texture) override;
	virtual void SetVertexBuffer(const Buffer* vbo, uint32_t vertexStride) override;
	virtual void SetIndexBuffer(const Buffer* ibo) override;
	virtual void CopyBufferToTexture(Buffer* buffer, Texture* texture) override;

	ID3D12CommandAllocator* CmdAllocator;
	ComPtr<ID3D12GraphicsCommandList> CmdList;
	ComPtr<ID3D12DescriptorHeap> SRVHeap;
	uint32_t NextSlot = 0;
};

}
}
}

#endif
