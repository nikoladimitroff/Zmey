#pragma once

#include <Zmey/Graphics/FrameData.h>
#include <Zmey/Graphics/RenderPasses.h>
#include <Zmey/Graphics/View.h>
#include <Zmey/Graphics/Backend/BackendDeclarations.h>

namespace Zmey
{

class World;
namespace Graphics
{

struct RendererData;
namespace Features
{
namespace UIRenderer
{
void GatherData(FrameData& frameData, World& world);
void PrepareData(FrameData& frameData, RendererData& data);
void GenerateCommands(FrameData& frameData, RenderPass pass, ViewType view, Backend::CommandList* cmdList, RendererData& data);
};
}
}
}