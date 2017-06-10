#pragma once

#include <Zmey/Graphics/Features/FEATURE_NAME.h>
#include <Zmey/Graphics/Backend/CommandList.h>
#include <Zmey/Graphics/Renderer.h>
#include <Zmey/World.h>
#include <Zmey/Components/TransformManager.h>

namespace Zmey
{
namespace Graphics
{
namespace Features
{

void FEATURE_NAME::GatherData(FrameData& frameData, World& world)
{

}

void FEATURE_NAME::PrepareData(FrameData& frameData, RendererData& data)
{

}

void FEATURE_NAME::GenerateCommands(FrameData& frameData, RenderPass pass, ViewType view, Backend::CommandList* list, RendererData& data)
{

}

}
}
}