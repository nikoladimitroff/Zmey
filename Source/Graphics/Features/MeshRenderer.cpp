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

	for (auto i = 0u; i < frameData.MeshHandles.size(); ++i)
	{
		auto mesh = data.MeshManager.GetMesh(frameData.MeshHandles[i]);
		assert(mesh);
		auto vbo = data.BufferManager.GetBuffer(mesh->VertexBuffer);
		auto ibo = data.BufferManager.GetBuffer(mesh->IndexBuffer);
		assert(vbo && ibo);

		list->SetVertexBuffer(vbo, sizeof(Vector3));
		list->SetIndexBuffer(ibo);

		list->Draw(mesh->IndexCount, 1, 0, 0);
	}
}

}
}
}