#include <Zmey/Logging.h>
#include <Zmey/Memory/MemoryManagement.h> // For TempAllocator
#include <Zmey/Graphics/FrameData.h>
#include <Zmey/Graphics/Renderer.h>

#include <Zmey/Graphics/Features.h>

#include <Zmey/Graphics/Backend/Backend.h>
#include <Zmey/Graphics/Backend/CommandList.h>

namespace Zmey
{
namespace Graphics
{

bool RendererInterface::CreateWindowSurface(WindowHandle handle)
{
	m_Backend->Initialize(handle);

	m_MainRenderPass = m_Backend->CreateRenderPass();
	m_RectsPipelineState = m_Backend->CreatePipelineState(m_MainRenderPass);

	auto swapChainCount = m_Backend->GetSwapChainBuffers();
	m_SwapChainFramebuffers.reserve(swapChainCount);
	m_CommandLists.reserve(swapChainCount);
	for (auto i = 0u; i < swapChainCount; ++i)
	{
		m_SwapChainFramebuffers.push_back(m_Backend->CreateFramebuffer(m_Backend->GetSwapChainImageView(i), m_MainRenderPass));
		m_CommandLists.push_back(m_Backend->CreateCommandList());
	}

	m_ImageAvailableSemaphore = m_Backend->CreateCommandListSemaphore();
	m_RenderFinishedSemaphore = m_Backend->CreateCommandListSemaphore();

	return true;
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
	m_CommandLists[imageIndex]->BeginRenderPass(m_MainRenderPass, m_SwapChainFramebuffers[imageIndex]);

	Features::MeshRenderer::GenerateCommands(frameData,
		RenderPass::Main,
		ViewType::PlayerView,
		m_CommandLists[imageIndex]);

	Features::RectRenderer::GenerateCommands(frameData,
		RenderPass::Main,
		ViewType::PlayerView,
		m_CommandLists[imageIndex],
		m_RectsPipelineState);

	m_CommandLists[imageIndex]->EndRenderPass(m_MainRenderPass, m_SwapChainFramebuffers[imageIndex]);
	// End Main pass on PlayerView

	m_CommandLists[imageIndex]->EndRecording();

	m_Backend->SubmitCommandList(m_CommandLists[imageIndex], m_ImageAvailableSemaphore, m_RenderFinishedSemaphore);
}

void RendererInterface::Present(FrameData& frameData, uint32_t imageIndex)
{
	m_Backend->Present(m_RenderFinishedSemaphore, imageIndex);
}

void RendererInterface::RenderFrame(FrameData& frameData)
{
	uint32_t imageIndex = m_Backend->AcquireNextSwapChainImage(m_ImageAvailableSemaphore);

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
{
	m_Backend.reset(Backend::CreateBackend());
}
}
}