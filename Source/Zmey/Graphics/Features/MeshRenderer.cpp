#pragma once

#include <Zmey/Graphics/Features/MeshRenderer.h>
#include <Zmey/Graphics/Backend/CommandList.h>
#include <Zmey/Graphics/Renderer.h>
#include <Zmey/World.h>
#include <Zmey/Components/TransformManager.h>
#include <Zmey/Components/MeshComponentManager.h>
#include <Zmey/Modules.h>

namespace Zmey
{
namespace Graphics
{
namespace Features
{

namespace
{
struct MeshRendererGatherJobData
{
	MeshHandle& HandleSlot;
	Matrix4x4& MatrixSlot;

	Components::TransformManager& TransformManager;
	EntityId Id;
	MeshHandle Handle;
};

void GatherDataEntryPoint(void* param)
{
	auto data = (MeshRendererGatherJobData*)param;

	data->HandleSlot = data->Handle;
	const auto& transform = data->TransformManager.Lookup(data->Id);

	data->MatrixSlot =
		glm::translate(transform.Position()) *
		glm::toMat4(transform.Rotation()) *
		glm::scale(transform.Scale());
}
}

void MeshRenderer::GatherData(FrameData& frameData, World& world)
{
	auto& meshManager = world.GetManager<Components::MeshComponentManager>();
	const auto& meshes = meshManager.GetMeshes();
	frameData.MeshHandles.resize(meshes.size());
	frameData.MeshTransforms.resize(meshes.size());
	auto& transformManager = world.GetManager<Components::TransformManager>();

	TEMP_ALLOCATOR_SCOPE;
	tmp::vector<Job::JobDecl> jobs;
	tmp::vector<MeshRendererGatherJobData> jobsData;
	jobs.reserve(meshes.size());
	jobsData.reserve(meshes.size());

	for (auto i = 0u; i < meshes.size(); ++i)
	{
		jobsData.push_back(MeshRendererGatherJobData{ frameData.MeshHandles[i], frameData.MeshTransforms[i], transformManager, std::get<0>(meshes[i]), std::get<1>(meshes[i])});
		jobs.push_back(Job::JobDecl{ GatherDataEntryPoint, &jobsData[i] });
	}

	Job::Counter counter;
	Modules.JobSystem.RunJobs("Mesh Gather Data", jobs.data(), uint32_t(jobs.size()), &counter);
	Modules.JobSystem.WaitForCounter(&counter, 0);
}

void MeshRenderer::PrepareData(FrameData& frameData, RendererData& data)
{

}

void MeshRenderer::GenerateCommands(FrameData& frameData, RenderPass pass, ViewType view, Backend::CommandList* list, RendererData& data)
{
	if (view != ViewType::PlayerView || pass != RenderPass::Main)
	{
		return;
	}

	list->BindGraphicsPipelineState(data.MeshesPipelineState);
	auto viewProjection = frameData.ProjectionMatrix * frameData.ViewMatrix;

	Vector3 lightDir(-1.0, -1.0, 1.0);
	lightDir = glm::normalize(lightDir);
	list->SetResourceSetData<ResourceSetType::Light>(data.MeshesPipelineState, lightDir, frameData.EyePosition);

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
		list->SetResourceSetData<ResourceSetType::Transform>(data.MeshesPipelineState, mat, frameData.MeshTransforms[i]);

		auto material = data.MaterialManager.GetMaterial(mesh->Material);
		list->SetResourceSetData<ResourceSetType::Material>(data.MeshesPipelineState, material);

		list->DrawIndexed(mesh->IndexCount, 1, 0, 0, 0);
	}
}

}
}
}