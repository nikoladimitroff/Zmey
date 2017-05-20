#pragma once

#include <stdint.h>
#include <Zmey/Graphics/Backend/BackendDeclarations.h>

namespace Zmey
{
namespace Graphics
{
namespace Backend
{

class CommandList
{
public:
	virtual void BeginRecording() = 0;
	virtual void EndRecording() = 0;

	virtual void BeginRenderPass(RenderPass* pass, Framebuffer* fb) = 0;
	virtual void EndRenderPass() = 0;

	virtual void BindPipelineState(PipelineState* state) = 0;

	virtual void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t startVertex, uint32_t startInstance) = 0;

	virtual void SetPushConstants(PipelineState* layout, uint32_t offset, uint32_t count, const void* data) = 0;
};

}
}
}
