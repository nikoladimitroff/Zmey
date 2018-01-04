#include <Zmey/Graphics/Backend/Dx12/Dx12CommandList.h>

#ifdef USE_DX12

#include <Zmey/Graphics/Backend/Dx12/Dx12Device.h>
#include <Zmey/Graphics/Backend/Dx12/Dx12Texture.h>
#include <Zmey/Graphics/RendererGlobals.h>

namespace Zmey
{
namespace Graphics
{
namespace Backend
{

void Dx12CommandList::BeginRecording()
{
	CmdList->Reset(CmdAllocator, nullptr);
	NextSlot = 0;

	CmdList->SetDescriptorHeaps(1, SRVHeap.GetAddressOf());
}

void Dx12CommandList::EndRecording()
{
	CmdList->Close();
}

void Dx12CommandList::BeginRenderPass(Framebuffer* fb, bool clear)
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

	if (clear)
	{
		const float clearColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		CmdList->ClearRenderTargetView(dxfb->RTV, clearColor, 0, nullptr);

		CmdList->ClearDepthStencilView(dxfb->DSV, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	}

	CmdList->OMSetRenderTargets(1, &dxfb->RTV, FALSE, &dxfb->DSV);

	D3D12_VIEWPORT viewport;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = float(fb->Width);
	viewport.Height = float(fb->Height);
	viewport.MinDepth = 0;
	viewport.MaxDepth = 1;
	CmdList->RSSetViewports(1, &viewport);

	D3D12_RECT scissor;
	scissor.left = 0;
	scissor.top = 0;
	scissor.right = fb->Width;
	scissor.bottom = fb->Height;
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

void Dx12CommandList::SetScissor(float x, float y, float width, float height)
{
	D3D12_RECT scissor;
	scissor.left = LONG(x);
	scissor.top = LONG(y);
	scissor.right = LONG(width);
	scissor.bottom = LONG(height);
	CmdList->RSSetScissorRects(1, &scissor);
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

void Dx12CommandList::DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance)
{
	CmdList->DrawIndexedInstanced(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void Dx12CommandList::SetPushConstants(GraphicsPipelineState* layout, uint32_t offset, uint32_t count, const void* data)
{
	CmdList->SetGraphicsRoot32BitConstants(0, count / sizeof(float), data, offset / sizeof(float));
}

void Dx12CommandList::SetShaderResourceView(GraphicsPipelineState* layout, Texture* texture)
{
	assert(NextSlot < 1024); // TODO: This is the random number for size of the heap for now
	ID3D12Device* device;
	CmdList->GetDevice(IID_PPV_ARGS(&device));

	D3D12_SHADER_RESOURCE_VIEW_DESC desc{};
	desc.Format = PixelFormatToDx12(texture->Format);
	desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	desc.Texture2D.MipLevels = 1;
	desc.Texture2D.MostDetailedMip = 0;

	D3D12_CPU_DESCRIPTOR_HANDLE handle;
	handle.ptr = SRVHeap->GetCPUDescriptorHandleForHeapStart().ptr + NextSlot * device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
	gpuHandle.ptr = SRVHeap->GetGPUDescriptorHandleForHeapStart().ptr + NextSlot * device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	++NextSlot;

	device->CreateShaderResourceView(reinterpret_cast<Dx12Texture*>(texture)->Texture.Get(), &desc, handle);

	CmdList->SetGraphicsRootDescriptorTable(1, gpuHandle);
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

void Dx12CommandList::CopyBufferToTexture(Buffer* buffer, Texture* texture)
{
	auto dx12Buffer = reinterpret_cast<Dx12Buffer*>(buffer);
	auto dx12Texture = reinterpret_cast<Dx12Texture*>(texture);

	{
		D3D12_RESOURCE_BARRIER barrier;
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = dx12Texture->Texture.Get();
		barrier.Transition.StateBefore = dx12Texture->State;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		CmdList->ResourceBarrier(1, &barrier);
	}

	ID3D12Device* device;
	CmdList->GetDevice(IID_PPV_ARGS(&device));

	D3D12_RESOURCE_DESC desc;
	desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	desc.Alignment = 0;
	desc.Width = texture->Width;
	desc.Height = texture->Height;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.Format = PixelFormatToDx12(texture->Format);
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	desc.Flags = D3D12_RESOURCE_FLAG_NONE;
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint;
	device->GetCopyableFootprints(&desc, 0, 1, 0, &footprint, nullptr, nullptr, nullptr);

	D3D12_TEXTURE_COPY_LOCATION dst;
	dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	dst.pResource = dx12Texture->Texture.Get();
	dst.SubresourceIndex = 0;

	D3D12_TEXTURE_COPY_LOCATION src;
	src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	src.pResource = dx12Buffer->Buffer;
	src.PlacedFootprint = footprint;

	CmdList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

	{
		D3D12_RESOURCE_BARRIER barrier;
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = dx12Texture->Texture.Get();
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		barrier.Transition.StateAfter = dx12Texture->State;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		CmdList->ResourceBarrier(1, &barrier);
	}
}

}
}
}


#endif