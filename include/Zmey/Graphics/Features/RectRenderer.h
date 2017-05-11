#pragma once

#include <Zmey/Graphics/FrameData.h>
#include <Zmey/Graphics/RenderPasses.h>
#include <Zmey/Graphics/View.h>
#include <Zmey/Memory/MemoryManagement.h>

namespace Zmey
{
namespace Graphics
{
struct RendererData; // TODO: remove this and use some kind of managers for state
namespace Features
{
namespace RectRenderer
{
void GatherData(FrameData& frameData, Rect* rects, uint64_t count);
void PrepareData(FrameData& frameData);
void GenerateCommands(FrameData& frameData, RenderPass pass, ViewType view, void* cmdBuffer, RendererData& data); // TODO: don't use void* for commands
};
}
}
}