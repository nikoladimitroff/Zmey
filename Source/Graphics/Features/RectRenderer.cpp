#pragma once

#include <Zmey/Graphics/Features/RectRenderer.h>
#include <Zmey/Graphics/Backend/CommandList.h>
#include <Zmey/Graphics/Renderer.h>

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
	values[1] = -(rect.y * 2.0f - 1.0f);
	values[2] = rect.width * 2.0f;
	values[3] = rect.height * 2.0f;
}
}

void RectRenderer::GenerateCommands(FrameData& frameData, RenderPass pass, ViewType view, Backend::CommandList* list, RendererData& data)
{
	if (view == ViewType::PlayerView
		&& pass == RenderPass::Main)
	{
		list->BindPipelineState(data.RectsPipelineState, true);
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

		list->SetPushConstants(data.RectsPipelineState, 0, 4 * sizeof(float), rectValues);
		list->SetPushConstants(data.RectsPipelineState, 16, 4 * sizeof(float), rect.color);

		list->Draw(4, 1, 0, 0);
	}
}

}
}
}