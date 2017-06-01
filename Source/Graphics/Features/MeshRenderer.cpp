#pragma once

#include <Zmey/Graphics/Features/MeshRenderer.h>
#include <Zmey/Graphics/Backend/CommandList.h>
#include <Zmey/Graphics/Renderer.h>

namespace Zmey
{
namespace Graphics
{
namespace Features
{

void MeshRenderer::GatherData(FrameData& frameData, MeshHandle handle)
{
	frameData.MeshHandles.push_back(handle);
	// TODO(alex): remove test code
	Matrix4x4 meshTransform = glm::translate(Vector3(0.0f, 50.0f, 20.0f)) * glm::rotate(glm::radians(-90.0f), Vector3(1.0f, 0.0f, 0.0f));
	frameData.MeshTransforms.push_back(meshTransform);
}

void MeshRenderer::PrepareData(FrameData& frameData, RendererData& data)
{

}

void MeshRenderer::GenerateCommands(FrameData& frameData, RenderPass pass, ViewType view, Backend::CommandList* list, RendererData& data)
{
	if (view == ViewType::PlayerView
		&& pass == RenderPass::Main)
	{
		list->BindPipelineState(data.MeshesPipelineState);
	}
	else
	{
		assert(false);
	}

	auto viewProjection = frameData.ProjectionMatrix * frameData.ViewMatrix;

	for (auto i = 0u; i < frameData.MeshHandles.size(); ++i)
	{
		auto mesh = data.MeshManager.GetMesh(frameData.MeshHandles[i]);
		assert(mesh);
		auto vbo = data.BufferManager.GetBuffer(mesh->VertexBuffer);
		auto ibo = data.BufferManager.GetBuffer(mesh->IndexBuffer);
		assert(vbo && ibo);

		list->SetVertexBuffer(vbo, sizeof(MeshVertex));
		list->SetIndexBuffer(ibo);

		auto mat = viewProjection * frameData.MeshTransforms[i];
		list->SetPushConstants(data.MeshesPipelineState, 0, sizeof(Matrix4x4), &mat);

		list->Draw(mesh->IndexCount, 1, 0, 0);
	}
}

}
}
}