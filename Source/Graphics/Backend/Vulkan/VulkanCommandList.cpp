#include <Zmey/Graphics/Backend/Vulkan/VulkanCommandList.h>
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

void VulkanCommandList::BeginRenderPass(RenderPass* pass, Framebuffer* fb)
{
	VkRenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = reinterpret_cast<VulkanRenderPass*>(pass)->RenderPass;
	renderPassInfo.framebuffer = reinterpret_cast<VulkanFramebuffer*>(fb)->Framebuffer;
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = { 1264, 681 };

	VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 0.0f };
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;

	vkCmdBeginRenderPass(CmdBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void VulkanCommandList::EndRenderPass(RenderPass* pass, Framebuffer* fb)
{
	vkCmdEndRenderPass(CmdBuffer);
}

void VulkanCommandList::BindPipelineState(PipelineState* state)
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

}
}
}
