#pragma once

#include <Zmey/Graphics/FrameData.h>
#include <Zmey/Graphics/RenderPasses.h>
#include <Zmey/Graphics/Backend/BackendDeclarations.h>
#include <Zmey/Graphics/View.h>
#include <Zmey/Memory/MemoryManagement.h>

namespace Zmey
{
namespace Graphics
{

struct RendererData;
namespace Features
{
namespace RectRenderer
{
void GatherData(FrameData& frameData, Rect* rects, uint64_t count);
void PrepareData(FrameData& frameData);
void GenerateCommands(FrameData& frameData, RenderPass pass, ViewType view, Backend::CommandList* cmdList, RendererData& data);
};
}
}
}