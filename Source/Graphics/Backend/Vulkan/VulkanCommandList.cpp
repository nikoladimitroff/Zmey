#include <Zmey/Graphics/Backend/Vulkan/VulkanCommandList.h>
#ifdef USE_VULKAN

#include <Zmey/Graphics/Backend/Vulkan/VulkanDevice.h>
#include <Zmey/Graphics/Backend/Vulkan/VulkanBuffer.h>
#include <Zmey/Graphics/Backend/Vulkan/VulkanTexture.h>
#include <Zmey/Graphics/RendererGlobals.h>

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

	auto device = reinterpret_cast<VulkanDevice*>(Globals::g_Device)->GetNativeDevice();
	vkResetDescriptorPool(device, Pool, 0);
}

void VulkanCommandList::EndRecording()
{
	if (vkEndCommandBuffer(CmdBuffer) != VK_SUCCESS)
	{
		LOG(Error, Vulkan, "Failed to record command buffer!");
	}
}

void VulkanCommandList::BeginRenderPass(Framebuffer* fb, bool clear)
{
	VkRenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = reinterpret_cast<VulkanFramebuffer*>(fb)->RenderPass;
	renderPassInfo.framebuffer = reinterpret_cast<VulkanFramebuffer*>(fb)->Framebuffer;
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = { fb->Width, fb->Height };

	VkClearValue clearColor[2];
	clearColor[0].color = { 1.0f, 1.0f, 1.0f, 1.0f };
	clearColor[1].depthStencil = { 1.0f, 0 };
	renderPassInfo.clearValueCount = 2;
	renderPassInfo.pClearValues = clearColor;

	vkCmdBeginRenderPass(CmdBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	// We are using VK_KHR_maintenance1 for negative sized viewport in order to be consistent with Dx12 Viewport
	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = float(fb->Height);
	viewport.width = float(fb->Width);
	viewport.height = -float(fb->Height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(CmdBuffer, 0, 1, &viewport);

	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent.width = fb->Width;
	scissor.extent.height = fb->Height;
	vkCmdSetScissor(CmdBuffer, 0, 1, &scissor);
}

void VulkanCommandList::EndRenderPass(Framebuffer* fb)
{
	vkCmdEndRenderPass(CmdBuffer);
}

void VulkanCommandList::SetScissor(float x, float y, float width, float height)
{
	VkRect2D scissor = {};
	scissor.offset = { int32_t(x), int32_t(y) };
	scissor.extent.width = uint32_t(width);
	scissor.extent.height = uint32_t(height);
	vkCmdSetScissor(CmdBuffer, 0, 1, &scissor);
}

void VulkanCommandList::BindGraphicsPipelineState(GraphicsPipelineState* state)
{
	vkCmdBindPipeline(CmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, reinterpret_cast<VulkanPipelineState*>(state)->Pipeline);
}

void VulkanCommandList::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t startVertex, uint32_t startInstance)
{
	vkCmdDraw(CmdBuffer, vertexCount, instanceCount, startVertex, startInstance);
}

void VulkanCommandList::DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance)
{
	vkCmdDrawIndexed(CmdBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void VulkanCommandList::SetPushConstants(GraphicsPipelineState* layout, uint32_t offset, uint32_t count, const void* data)
{
	vkCmdPushConstants(CmdBuffer,
		reinterpret_cast<VulkanPipelineState*>(layout)->Layout,
		VK_SHADER_STAGE_ALL_GRAPHICS,
		offset,
		count,
		data);
}

void VulkanCommandList::SetShaderResourceView(GraphicsPipelineState* layout, Texture* texture)
{
	auto device = reinterpret_cast<VulkanDevice*>(Globals::g_Device)->GetNativeDevice();

	auto vkPipeline = reinterpret_cast<VulkanPipelineState*>(layout);
	auto vkTexture = reinterpret_cast<VulkanTexture*>(texture);

	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = Pool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &vkPipeline->SetLayout;

	VkDescriptorSet set;
	if (vkAllocateDescriptorSets(device, &allocInfo, &set) != VK_SUCCESS)
	{
		LOG(Error, Vulkan, "failed to allocate descriptor set!");
	}

	VkDescriptorImageInfo imageInfo = {};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = vkTexture->View;

	VkWriteDescriptorSet descriptorWrite = {};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = set;
	descriptorWrite.dstBinding = 0;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pBufferInfo = nullptr;
	descriptorWrite.pImageInfo = &imageInfo;
	descriptorWrite.pTexelBufferView = nullptr;

	vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);

	vkCmdBindDescriptorSets(CmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipeline->Layout, 0, 1, &set, 0, nullptr);
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

void VulkanCommandList::CopyBufferToTexture(Buffer* buffer, Texture* texture)
{
	auto vkBuffer = reinterpret_cast<VulkanBuffer*>(buffer);
	auto vkTexture = reinterpret_cast<VulkanTexture*>(texture);

	{
		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = vkTexture->Layout;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = vkTexture->TextureHandle;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		vkCmdPipelineBarrier(CmdBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
	}

	VkBufferImageCopy region = {};
	region.bufferOffset = 0;
	region.bufferRowLength = vkTexture->Width;
	region.bufferImageHeight = vkTexture->Height;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset.x = 0;
	region.imageOffset.y = 0;
	region.imageOffset.z = 0;
	region.imageExtent.width = vkTexture->Width;
	region.imageExtent.height = vkTexture->Height;
	region.imageExtent.depth = 1;

	vkCmdCopyBufferToImage(CmdBuffer, vkBuffer->UploadBufferHandle, vkTexture->TextureHandle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	{
		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = vkTexture->TextureHandle;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		vkCmdPipelineBarrier(CmdBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
	}

	vkTexture->Layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
}

}
}
}

#endif