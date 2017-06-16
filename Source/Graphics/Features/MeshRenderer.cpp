#pragma once

#include <Zmey/Graphics/Features/MeshRenderer.h>
#include <Zmey/Graphics/Backend/CommandList.h>
#include <Zmey/Graphics/Renderer.h>
#include <Zmey/World.h>
#include <Zmey/Components/TransformManager.h>
#include <Zmey/Components/MeshComponentManager.h>

namespace Zmey
{
namespace Graphics
{
namespace Features
{

void MeshRenderer::GatherData(FrameData& frameData, World& world)
{
	auto& meshManager = world.GetManager<Components::MeshComponentManager>();
	const auto& meshes = meshManager.GetMeshes();
	frameData.MeshHandles.reserve(meshes.size());
	frameData.MeshTransforms.reserve(meshes.size());
	auto& transformManager = world.GetManager<Components::TransformManager>();
	for (const auto& mesh : meshes)
	{
		frameData.MeshHandles.push_back(mesh.second);
		const auto& transform = transformManager.Lookup(mesh.first);

		frameData.MeshTransforms.push_back(
			glm::translate(transform.Position()) *
			glm::toMat4(transform.Rotation()) *
			glm::scale(transform.Scale())
		);
	}
}

void MeshRenderer::PrepareData(FrameData& frameData, RendererData& data)
{

}

void MeshRenderer::GenerateCommands(FrameData& frameData, RenderPass pass, ViewType view, Backend::CommandList* list, RendererData& data)
{
	if (view == ViewType::PlayerView
		&& pass == RenderPass::Main)
	{
		list->BindPipelineState(data.MeshesPipelineState, false);
	}
	else
	{
		assert(false);
	}

	auto viewProjection = frameData.ProjectionMatrix * frameData.ViewMatrix;

	Vector3 lightDir(-1.0, -1.0, 1.0);
	lightDir = glm::normalize(lightDir);
	list->SetPushConstants(data.MeshesPipelineState, 2 * sizeof(Matrix4x4), sizeof(Vector3), &lightDir);
	list->SetPushConstants(data.MeshesPipelineState, 2 * sizeof(Matrix4x4) + sizeof(Vector3), sizeof(Vector3), &frameData.EyePosition);

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
		list->SetPushConstants(data.MeshesPipelineState, sizeof(Matrix4x4), sizeof(Matrix4x4), &frameData.MeshTransforms[i]);

		list->Draw(mesh->IndexCount, 1, 0, 0);
	}
}

}
}
}