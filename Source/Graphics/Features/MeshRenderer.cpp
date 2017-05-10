#pragma once

#include <Zmey/Graphics/Features/MeshRenderer.h>
#include "../RendererData.h"
#include "../VulkanHelpers.h"

namespace Zmey
{
namespace Graphics
{
namespace Features
{

void MeshRenderer::GatherData(FrameData& frameData)
{
	// TODO(alex): Test code remove me
	frameData.MeshHandles.push_back(0);
	frameData.MeshPositions.push_back(Vector3(0.0f, 0.0f, 0.0f));
}

void MeshRenderer::PrepareData(FrameData& frameData)
{
	// No prepare for now
}

void MeshRenderer::GenerateCommands(FrameData& frameData, RenderPass pass, ViewType view, void* cmdBuffer, RendererData& data)
{
	auto cmd = reinterpret_cast<VkCommandBuffer>(cmdBuffer);
	
	if (view == ViewType::PlayerView
		&& pass == RenderPass::Main)
	{
		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, data.m_GraphicsPipeline);
	}
	else
	{
		assert(false);
	}

	for (auto i = 0u; i < frameData.MeshHandles.size(); ++i)
	{
		// TODO: use real handle and real position
		VkBuffer vertexBuffers[] = { data.m_VertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);

		vkCmdDraw(cmd, 4, 1, 0, 0);
	}
}

}
}
}