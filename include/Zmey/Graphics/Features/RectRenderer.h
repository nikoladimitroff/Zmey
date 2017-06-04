#pragma once

#include <Zmey/Graphics/FrameData.h>
#include <Zmey/Graphics/RenderPasses.h>
#include <Zmey/Graphics/Backend/BackendDeclarations.h>
#include <Zmey/Graphics/View.h>
#include <Zmey/Memory/MemoryManagement.h>

namespace Zmey
{

class World;
namespace Graphics
{

struct RendererData;
namespace Features
{
namespace RectRenderer
{
void GatherData(FrameData& frameData, World& world); //TODO(alex): make this const after there managers can be getted with constnest
void PrepareData(FrameData& frameData);
void GenerateCommands(FrameData& frameData, RenderPass pass, ViewType view, Backend::CommandList* cmdList, RendererData& data);
};
}
}
}