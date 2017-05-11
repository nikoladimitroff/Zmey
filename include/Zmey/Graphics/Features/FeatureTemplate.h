#pragma once

#include <Zmey/Graphics/FrameData.h>
#include <Zmey/Graphics/RenderPasses.h>
#include <Zmey/Graphics/View.h>

namespace Zmey
{
namespace Graphics
{
struct RendererData; // TODO: remove this and use some kind of managers for state
namespace Features
{
namespace FEATURE_NAME
{
void GatherData(FrameData& frameData); // TODO: add world state
void PrepareData(FrameData& frameData);
void GenerateCommands(FrameData& frameData, RenderPass pass, ViewType view, void* cmdBuffer, RendererData& data); // TODO: don't use void* for commands
};
}
}
}