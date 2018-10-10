#pragma once

#include <Zmey/Graphics/Features/UIRenderer.h>
#include <Zmey/Graphics/Backend/CommandList.h>
#include <Zmey/Graphics/Renderer.h>
#include <Zmey/World.h>
#include <Zmey/Components/TransformManager.h>
#include <Zmey/Graphics/Backend/Buffer.h>
#include <Zmey/Graphics/Backend/Device.h>
#include <Zmey/Graphics/RendererGlobals.h>

#include <imgui/imgui.h>

namespace Zmey
{
namespace Graphics
{
namespace Features
{

void UIRenderer::GatherData(FrameData& frameData, World& world)
{
	ImGui::Render();
	auto drawData = ImGui::GetDrawData();
	if (drawData->Valid)
	{
		// Clear old data as we are reusing this vectors for now
		// TODO: Fix me
		frameData.UIDrawVertexOffset.clear();

		frameData.UIVertexData.resize(drawData->TotalVtxCount * sizeof(ImDrawVert));
		frameData.UIIndexData.resize(drawData->TotalIdxCount * sizeof(ImDrawIdx));
		uint32_t totalDrawDataCount = 0u;

		auto vertexMemory = reinterpret_cast<ImDrawVert*>(frameData.UIVertexData.data());
		auto indexMemory = reinterpret_cast<ImDrawIdx*>(frameData.UIIndexData.data());
		for (int i = 0; i < drawData->CmdListsCount; ++i)
		{
			const ImDrawList* cmdList = drawData->CmdLists[i];
			memcpy(vertexMemory, cmdList->VtxBuffer.Data, cmdList->VtxBuffer.Size * sizeof(ImDrawVert));
			memcpy(indexMemory, cmdList->IdxBuffer.Data, cmdList->IdxBuffer.Size * sizeof(ImDrawIdx));
			vertexMemory += cmdList->VtxBuffer.Size;
			indexMemory += cmdList->IdxBuffer.Size;

			totalDrawDataCount += cmdList->CmdBuffer.Size;
		}

		frameData.UIDrawData.resize(totalDrawDataCount * sizeof(ImDrawCmd));
		frameData.UIDrawVertexOffset.reserve(totalDrawDataCount);
		auto drawDataMemory = reinterpret_cast<ImDrawCmd*>(frameData.UIDrawData.data());
		uint32_t vertexOffset = 0;
		for (int i = 0; i < drawData->CmdListsCount; ++i)
		{
			const ImDrawList* cmdList = drawData->CmdLists[i];

			memcpy(drawDataMemory, cmdList->CmdBuffer.Data, cmdList->CmdBuffer.Size * sizeof(ImDrawCmd));
			drawDataMemory += cmdList->CmdBuffer.Size;

			std::fill_n(std::back_inserter(frameData.UIDrawVertexOffset), cmdList->CmdBuffer.Size, vertexOffset);
			vertexOffset += cmdList->VtxBuffer.Size;
		}
	}
}

void UIRenderer::PrepareData(FrameData& frameData, RendererData& data)
{
	if (frameData.UIDrawData.empty())
	{
		return;
	}

	const auto currentIndex = frameData.FrameIndex % 2;

	// Resize buffers if needed
	auto vertexDataSize = uint32_t(frameData.UIVertexData.size());
	if (!data.UIData.VertexBuffers[currentIndex]
		|| data.UIData.VertexBuffers[currentIndex]->Size < vertexDataSize)
	{
		if (data.UIData.VertexBuffers[currentIndex])
		{
			Globals::g_Device->DestroyBuffer(data.UIData.VertexBuffers[currentIndex]);
		}
		data.UIData.VertexBuffers[currentIndex] = Globals::g_Device->CreateBuffer(vertexDataSize, Backend::BufferUsage::Vertex);
	}

	auto indexDataSize = uint32_t(frameData.UIIndexData.size());
	if (!data.UIData.IndexBuffers[currentIndex]
		|| data.UIData.IndexBuffers[currentIndex]->Size < indexDataSize)
	{
		if (data.UIData.IndexBuffers[currentIndex])
		{
			Globals::g_Device->DestroyBuffer(data.UIData.IndexBuffers[currentIndex]);
		}
		data.UIData.IndexBuffers[currentIndex] = Globals::g_Device->CreateBuffer(indexDataSize, Backend::BufferUsage::Index);
	}

	// Upload data
	auto vertexMemory = data.UIData.VertexBuffers[currentIndex]->Map();
	auto indexMemory = data.UIData.IndexBuffers[currentIndex]->Map();
	memcpy(vertexMemory, frameData.UIVertexData.data(), frameData.UIVertexData.size());
	memcpy(indexMemory, frameData.UIIndexData.data(), frameData.UIIndexData.size());
	data.UIData.VertexBuffers[currentIndex]->Unmap();
	data.UIData.IndexBuffers[currentIndex]->Unmap();
}

void UIRenderer::GenerateCommands(FrameData& frameData, RenderPass pass, ViewType view, Backend::CommandList* list, RendererData& data)
{
	if (pass != RenderPass::UI || view != ViewType::PlayerView)
	{
		return;
	}

	const auto currentIndex = frameData.FrameIndex % 2;

	list->BindGraphicsPipelineState(data.UIData.PipelineState);
	list->SetVertexBuffer(data.UIData.VertexBuffers[currentIndex], sizeof(ImDrawVert));
	list->SetIndexBuffer(data.UIData.IndexBuffers[currentIndex]);

	const auto swapChainSize = Globals::g_Device->GetSwapChainSize();

	Vector2 scale {
		2.0f / swapChainSize.x,
		-2.0f / swapChainSize.y
	};
	Vector2 translate {
		-1.0f,
		1.0f
	};
	list->SetResourceSetData<ResourceSetType::UIPosition>(data.UIData.PipelineState, scale, translate);

	int indexOffset = 0;
	TextureHandle currentHandle = -1;
	const auto drawDataPtr = reinterpret_cast<ImDrawCmd*>(frameData.UIDrawData.data());
	const auto drawDataSize = frameData.UIDrawData.size() / sizeof(ImDrawCmd);
	for (auto i = 0u; i < drawDataSize; ++i)
	{
		const ImDrawCmd* cmd = drawDataPtr + i;
		if (!cmd->UserCallback)
		{
			const auto texHandle = reinterpret_cast<TextureHandle>(cmd->TextureId);
			if (texHandle != currentHandle)
			{
				auto texture = data.TextureManager.GetTexture(texHandle);
				list->SetResourceSetData<ResourceSetType::UITexture>(data.UIData.PipelineState, texture);
				currentHandle = texHandle;
			}

			list->SetScissor(cmd->ClipRect.x, cmd->ClipRect.y, cmd->ClipRect.z, cmd->ClipRect.w);
			list->DrawIndexed(cmd->ElemCount, 1, indexOffset, frameData.UIDrawVertexOffset[i], 0);
		}
		else
		{
			NOT_REACHED();
		}
		indexOffset += cmd->ElemCount;
	}
}

}
}
}