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
	virtual void SetScissor(float x, float y, float width, float height) override;

	virtual void BindGraphicsPipelineState(GraphicsPipelineState* state) override;
	virtual void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t startVertex, uint32_t startInstance) override;
	virtual void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance) override;
	virtual void SetPushConstants(GraphicsPipelineState* layout, uint32_t offset, uint32_t count, const void* data) override;
	virtual void SetShaderResourceView(GraphicsPipelineState* layout, const Texture* texture) override;
	virtual void SetVertexBuffer(const Buffer* vbo, uint32_t vertexStride) override;
	virtual void SetIndexBuffer(const Buffer* ibo) override;
	virtual void CopyBufferToTexture(Buffer* buffer, Texture* texture) override;

	VkCommandBuffer CmdBuffer;
	VkDescriptorPool Pool;
};

}
}
}

#endif
