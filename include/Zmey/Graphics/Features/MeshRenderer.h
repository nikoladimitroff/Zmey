#pragma once

#include <Zmey/Graphics/FrameData.h>
#include <Zmey/Graphics/RenderPasses.h>
#include <Zmey/Graphics/View.h>
#include <Zmey/Graphics/Backend/BackendDeclarations.h>

namespace Zmey
{
namespace Graphics
{

struct RendererData;
namespace Features
{
namespace MeshRenderer
{
void GatherData(FrameData& frameData, MeshHandle handle); // TODO: add world state
void PrepareData(FrameData& frameData, RendererData& data);
void GenerateCommands(FrameData& frameData, RenderPass pass, ViewType view, Backend::CommandList* cmdList, RendererData& data);
};
}
}
}