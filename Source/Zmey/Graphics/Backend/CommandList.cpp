#pragma once

#include <Zmey/Graphics/Backend/CommandList.h>
#include <Zmey/Graphics/Managers/MaterialManager.h>
#include <Zmey/Graphics/Backend/GraphicsPipelineState.h>
#include <Zmey/Graphics/Backend/BackendResourceSet.h>
#include <Zmey/Modules.h>

namespace Zmey
{
namespace Graphics
{
namespace Backend
{
namespace
{
bool IsResourceSetInPipeline(GraphicsPipelineState* pipeline, ResourceSetType set)
{
	return std::find(
		pipeline->Desc.ResourceTable.ResourceSets.begin(),
		pipeline->Desc.ResourceTable.ResourceSets.end(),
		set) != pipeline->Desc.ResourceTable.ResourceSets.end();
}

uint32_t ResourceSetConstantBufferStartOffset(GraphicsPipelineState* pipeline, ResourceSetType set)
{
	const auto& sets = pipeline->Desc.ResourceTable.ResourceSets;
	auto findItr = std::find(sets.begin(), sets.end(), set);
	return std::accumulate(sets.begin(), findItr, uint32_t(0), [](uint32_t result, ResourceSetType set) {
		return result + uint32_t(GetPushConstantCountForResourceSet(set) * sizeof(uint32_t));
	});
}
}

// TODO: investigate why args cannot be const
// TODO: investigate why this function specialization is found
template<>
void CommandList::SetResourceSetData<ResourceSetType::Light>(GraphicsPipelineState* pipeline, Vector3& lightDir, Vector3& eyePosition)
{
	assert(IsResourceSetInPipeline(pipeline, ResourceSetType::Light));
	auto startOffset = ResourceSetConstantBufferStartOffset(pipeline, ResourceSetType::Light);
	SetPushConstants(pipeline, startOffset, sizeof(Vector3), &lightDir);
	SetPushConstants(pipeline, startOffset + sizeof(Vector4), sizeof(Vector3), &eyePosition);
}

template<>
void CommandList::SetResourceSetData<ResourceSetType::Transform>(GraphicsPipelineState* pipeline, Matrix4x4& wvp, Matrix4x4& world)
{
	assert(IsResourceSetInPipeline(pipeline, ResourceSetType::Transform));
	auto startOffset = ResourceSetConstantBufferStartOffset(pipeline, ResourceSetType::Transform);
	SetPushConstants(pipeline, startOffset, sizeof(Matrix4x4), &wvp);
	SetPushConstants(pipeline, startOffset + sizeof(Matrix4x4), sizeof(Matrix4x4), &world);
}

template<>
void CommandList::SetResourceSetData<ResourceSetType::Material>(GraphicsPipelineState* pipeline, const Material*& material)
{
	assert(IsResourceSetInPipeline(pipeline, ResourceSetType::Material));
	auto startOffset = ResourceSetConstantBufferStartOffset(pipeline, ResourceSetType::Material);
	SetPushConstants(pipeline, startOffset, sizeof(Vector3), &material->BaseColorFactor);

	uint32_t hasColorTexture = false;
	if (material->BaseColorTexture != TextureHandle(-1))
	{
		hasColorTexture = true;

		SetShaderResourceView(pipeline,
			Modules.Renderer.GetRendererData().TextureManager.GetTexture(material->BaseColorTexture));
	}

	SetPushConstants(pipeline, startOffset + sizeof(Vector3), sizeof(uint32_t), &hasColorTexture);
}

template<>
void CommandList::SetResourceSetData<ResourceSetType::UIPosition>(GraphicsPipelineState* pipeline, Vector2& scale, Vector2& translate)
{
	assert(IsResourceSetInPipeline(pipeline, ResourceSetType::UIPosition));
	auto startOffset = ResourceSetConstantBufferStartOffset(pipeline, ResourceSetType::UIPosition);
	SetPushConstants(pipeline, startOffset, sizeof(float) * 2, &scale);
	SetPushConstants(pipeline, startOffset + sizeof(float) * 2, sizeof(float) * 2, &translate);
}

template<>
void CommandList::SetResourceSetData<ResourceSetType::UITexture>(GraphicsPipelineState* pipeline, Texture*& texture)
{
	assert(IsResourceSetInPipeline(pipeline, ResourceSetType::UITexture));
	SetShaderResourceView(pipeline, texture);
}
}
}
}
