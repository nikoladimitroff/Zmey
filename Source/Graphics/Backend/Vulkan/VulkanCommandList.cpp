#include <Zmey/Graphics/Backend/Vulkan/VulkanCommandList.h>
#ifdef USE_VULKAN

#include <Zmey/Graphics/Backend/Vulkan/VulkanBackend.h>

namespace Zmey
{
namespace Graphics
{
namespace Backend
{

void VulkanCommandList::BeginRecording()
{
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	beginInfo.pInheritanceInfo = nullptr;

	// This will implicitly reset the buffer
	vkBeginCommandBuffer(CmdBuffer, &beginInfo);
}

void VulkanCommandList::EndRecording()
{
	if (vkEndCommandBuffer(CmdBuffer) != VK_SUCCESS)
	{
		LOG(Error, Vulkan, "Failed to record command buffer!");
	}
}

void VulkanCommandList::BeginRenderPass(Framebuffer* fb)
{
	VkRenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = reinterpret_cast<VulkanFramebuffer*>(fb)->RenderPass;
	renderPassInfo.framebuffer = reinterpret_cast<VulkanFramebuffer*>(fb)->Framebuffer;
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = { 1264, 681 };

	VkClearValue clearColor[2];
	clearColor[0].color = { 1.0f, 1.0f, 1.0f, 1.0f };
	clearColor[1].depthStencil = { 1.0f, 0 };
	renderPassInfo.clearValueCount = 2;
	renderPassInfo.pClearValues = clearColor;

	vkCmdBeginRenderPass(CmdBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void VulkanCommandList::EndRenderPass(Framebuffer* fb)
{
	vkCmdEndRenderPass(CmdBuffer);
}

void VulkanCommandList::BindPipelineState(PipelineState* state, bool strip)
{
	vkCmdBindPipeline(CmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, reinterpret_cast<VulkanPipelineState*>(state)->Pipeline);
}

void VulkanCommandList::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t startVertex, uint32_t startInstance)
{
	vkCmdDraw(CmdBuffer, vertexCount, instanceCount, startVertex, startInstance);
}

void VulkanCommandList::SetPushConstants(PipelineState* layout, uint32_t offset, uint32_t count, const void* data)
{
	vkCmdPushConstants(CmdBuffer,
		reinterpret_cast<VulkanPipelineState*>(layout)->Layout,
		VK_SHADER_STAGE_ALL_GRAPHICS,
		offset,
		count,
		data);
}

void VulkanCommandList::SetVertexBuffer(const Buffer* vbo, uint32_t vertexStride)
{
	auto buffer = reinterpret_cast<const VulkanBuffer*>(vbo);

	VkDeviceSize offset[] = { 0 };
	vkCmdBindVertexBuffers(CmdBuffer, 0, 1, &buffer->BufferHandle, offset);
}

void VulkanCommandList::SetIndexBuffer(const Buffer* ibo)
{
	auto buffer = reinterpret_cast<const VulkanBuffer*>(ibo);

	vkCmdBindIndexBuffer(CmdBuffer, buffer->BufferHandle, 0, VK_INDEX_TYPE_UINT32);
}

}
}
}

#endif