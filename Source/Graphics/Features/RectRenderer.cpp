#pragma once

#include <Zmey/Graphics/Features/RectRenderer.h>
#include "../RendererData.h"
#include "../VulkanHelpers.h"

namespace Zmey
{
namespace Graphics
{
namespace Features
{

void RectRenderer::GatherData(FrameData& frameData, Rect* rects, uint64_t count)
{
	frameData.RectsToDraw.reserve(count);
	std::copy(rects, rects + count, std::back_inserter(frameData.RectsToDraw));
}

void RectRenderer::PrepareData(FrameData& frameData)
{

}

namespace
{
	void ToScreenSpace(float values[4], Rect& rect)
	{
		values[0] = rect.x * 2.0f - 1.0f;
		values[1] = rect.y * 2.0f - 1.0f;
		values[2] = rect.width * 2.0f;
		values[3] = rect.height * 2.0f;
	}
}

void RectRenderer::GenerateCommands(FrameData& frameData, RenderPass pass, ViewType view, void* cmdBuffer, RendererData& data)
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

	for (auto i = 0u; i < frameData.RectsToDraw.size(); ++i)
	{
		auto& rect = frameData.RectsToDraw[i];
		float rectValues[4];
		ToScreenSpace(rectValues, rect);

		vkCmdPushConstants(cmd, data.m_PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, 4 * sizeof(float), rectValues);
		vkCmdPushConstants(cmd, data.m_PipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 16, 4 * sizeof(float), rect.color);

		vkCmdDraw(cmd, 4, 1, 0, 0);
	}
}

}
}
}