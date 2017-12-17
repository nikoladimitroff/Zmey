#pragma once

#include <Zmey/Config.h>
#ifdef USE_VULKAN

#include <stdint.h>
#include <Zmey/Graphics/Backend/CommandList.h>

#include <Zmey/Graphics/Backend/Vulkan/VulkanHelpers.h>

namespace Zmey
{
namespace Graphics
{
namespace Backend
{

class VulkanCommandList : public CommandList
{
public:
	virtual void BeginRecording() override;
	virtual void EndRecording() override;

	virtual void BeginRenderPass(Framebuffer* fb) override;
	virtual void EndRenderPass(Framebuffer* fb) override;

	virtual void BindPipelineState(PipelineState* state, bool strip) override;
	virtual void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t startVertex, uint32_t startInstance) override;
	virtual void SetPushConstants(PipelineState* layout, uint32_t offset, uint32_t count, const void* data) override;
	virtual void SetVertexBuffer(const Buffer* vbo, uint32_t vertexStride) override;
	virtual void SetIndexBuffer(const Buffer* ibo) override;

	VkCommandBuffer CmdBuffer;
};

}
}
}

#endif
