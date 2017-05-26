#include <Zmey/Logging.h>
#include <Zmey/Memory/MemoryManagement.h> // For TempAllocator
#include <Zmey/Graphics/FrameData.h>
#include <Zmey/Graphics/Renderer.h>

#include <Zmey/Graphics/Features.h>

#include <Zmey/Graphics/Backend/Backend.h>
#include <Zmey/Graphics/Backend/CommandList.h>

#include <assimp/scene.h>

namespace Zmey
{
namespace Graphics
{

bool RendererInterface::CreateWindowSurface(WindowHandle handle)
{
	m_Backend->Initialize(handle);

	m_RectsPipelineState = m_Backend->CreatePipelineState();

	auto swapChainCount = m_Backend->GetSwapChainBuffers();
	m_SwapChainFramebuffers.reserve(swapChainCount);
	m_CommandLists.reserve(swapChainCount);
	for (auto i = 0u; i < swapChainCount; ++i)
	{
		m_SwapChainFramebuffers.push_back(m_Backend->CreateFramebuffer(m_Backend->GetSwapChainImageView(i)));
		m_CommandLists.push_back(m_Backend->CreateCommandList());
	}

	return true;
}

void RendererInterface::Unitialize()
{
	for (auto& list : m_CommandLists)
	{
		m_Backend->DestroyCommandList(list);
	}

	m_Backend->DestroyPipelineState(m_RectsPipelineState);
	for (auto& rtv : m_SwapChainFramebuffers)
	{
		m_Backend->DestroyFramebuffer(rtv);
	}

	m_Backend.reset();
}

void RendererInterface::PrepareData(FrameData& frameData)
{
	// TODO: Prepare graphics data
	Features::MeshRenderer::PrepareData(frameData);
	Features::RectRenderer::PrepareData(frameData);
}

void RendererInterface::GenerateCommands(FrameData& frameData, uint32_t imageIndex)
{
	// TODO: High level render passes

	m_CommandLists[imageIndex]->BeginRecording();

	// Begin Main pass on PlayerView
	m_CommandLists[imageIndex]->BeginRenderPass(m_SwapChainFramebuffers[imageIndex]);

	Features::MeshRenderer::GenerateCommands(frameData,
		RenderPass::Main,
		ViewType::PlayerView,
		m_CommandLists[imageIndex]);

	Features::RectRenderer::GenerateCommands(frameData,
		RenderPass::Main,
		ViewType::PlayerView,
		m_CommandLists[imageIndex],
		m_RectsPipelineState);

	m_CommandLists[imageIndex]->EndRenderPass(m_SwapChainFramebuffers[imageIndex]);
	// End Main pass on PlayerView

	m_CommandLists[imageIndex]->EndRecording();

	m_Backend->SubmitCommandList(m_CommandLists[imageIndex]);
}

void RendererInterface::Present(FrameData& frameData, uint32_t imageIndex)
{
	m_Backend->Present(imageIndex);
}

void RendererInterface::RenderFrame(FrameData& frameData)
{
	uint32_t imageIndex = m_Backend->AcquireNextSwapChainImage();

	PrepareData(frameData);

	GenerateCommands(frameData, imageIndex);

	Present(frameData, imageIndex);

	LastCompletedFrame = frameData.FrameIndex;
}

bool RendererInterface::CheckIfFrameCompleted(uint64_t frameIndex)
{
	return frameIndex <= LastCompletedFrame;
}

RendererInterface::RendererInterface()
	: m_Backend(Backend::CreateBackend())
	, m_Data(m_Backend.get())
{
}

MeshHandle RendererInterface::MeshLoaded(const aiScene* mesh)
{
	Mesh newMesh;
	newMesh.VertexBuffer = m_Data.BufferManager.CreateBuffer(64);
	newMesh.IndexBuffer = m_Data.BufferManager.CreateBuffer(64);
	return m_Data.MeshManager.CreateMesh(newMesh);
}
}
}