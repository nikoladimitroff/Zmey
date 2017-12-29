#include <Zmey/Graphics/Backend/Dx12/Dx12CommandList.h>

#ifdef USE_DX12

#include <Zmey/Graphics/Backend/Dx12/Dx12Device.h>

namespace Zmey
{
namespace Graphics
{
namespace Backend
{

void Dx12CommandList::BeginRecording()
{
	CmdList->Reset(CmdAllocator, nullptr);
}

void Dx12CommandList::EndRecording()
{
	CmdList->Close();
}

void Dx12CommandList::BeginRenderPass(Framebuffer* fb)
{
	auto dxfb = reinterpret_cast<Dx12Framebuffer*>(fb);

	D3D12_RESOURCE_BARRIER barrier;
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = reinterpret_cast<Dx12Framebuffer*>(fb)->TextureResource;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	CmdList->ResourceBarrier(1, &barrier);

	const float clearColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	CmdList->ClearRenderTargetView(dxfb->RTV, clearColor, 0, nullptr);

	CmdList->ClearDepthStencilView(dxfb->DSV, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	CmdList->OMSetRenderTargets(1, &dxfb->RTV, FALSE, &dxfb->DSV);

	D3D12_VIEWPORT viewport;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = 1280;
	viewport.Height = 720;
	viewport.MinDepth = 0;
	viewport.MaxDepth = 1;
	CmdList->RSSetViewports(1, &viewport);

	D3D12_RECT scissor;
	scissor.left = 0;
	scissor.top = 0;
	scissor.right = 1280;
	scissor.bottom = 720;
	CmdList->RSSetScissorRects(1, &scissor);
}

void Dx12CommandList::EndRenderPass(Framebuffer* fb)
{
	D3D12_RESOURCE_BARRIER barrier;
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = reinterpret_cast<Dx12Framebuffer*>(fb)->TextureResource;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	CmdList->ResourceBarrier(1, &barrier);
}

namespace
{
	D3D12_PRIMITIVE_TOPOLOGY ToDx12Topology(PrimitiveTopology topology)
	{
		switch (topology)
		{
		case PrimitiveTopology::TriangleList:
			return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		default:
			NOT_REACHED();
			return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
		}
	}
}

void Dx12CommandList::BindGraphicsPipelineState(GraphicsPipelineState* state)
{
	auto dx12State = reinterpret_cast<Dx12GraphicsPipelineState*>(state);
	CmdList->SetPipelineState(dx12State->PipelineState);
	CmdList->SetGraphicsRootSignature(dx12State->RootSignature);
	CmdList->IASetPrimitiveTopology(ToDx12Topology(state->Desc.Topology));
}

void Dx12CommandList::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t startVertex, uint32_t startInstance)
{
	CmdList->DrawInstanced(vertexCount, instanceCount, startVertex, startInstance);
}

void Dx12CommandList::SetPushConstants(GraphicsPipelineState* layout, uint32_t offset, uint32_t count, const void* data)
{
	CmdList->SetGraphicsRoot32BitConstants(0, count / sizeof(float), data, offset / sizeof(float));
}

void Dx12CommandList::SetVertexBuffer(const Buffer* vbo, uint32_t vertexStride)
{
	auto dx12Buffer = reinterpret_cast<const Dx12Buffer*>(vbo);

	D3D12_VERTEX_BUFFER_VIEW view;
	view.BufferLocation = dx12Buffer->Buffer->GetGPUVirtualAddress();
	view.SizeInBytes = vbo->Size;
	view.StrideInBytes = vertexStride;

	CmdList->IASetVertexBuffers(0, 1, &view);
}

void Dx12CommandList::SetIndexBuffer(const Buffer* ibo)
{
	auto dx12Buffer = reinterpret_cast<const Dx12Buffer*>(ibo);

	D3D12_INDEX_BUFFER_VIEW view;
	view.BufferLocation = dx12Buffer->Buffer->GetGPUVirtualAddress();
	view.SizeInBytes = ibo->Size;
	view.Format = DXGI_FORMAT_R32_UINT;

	CmdList->IASetIndexBuffer(&view);
}

}
}
}


#endif