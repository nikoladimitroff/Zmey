#pragma once

#include <Zmey/Platform/Platform.h>
#include <Zmey/Memory/MemoryManagement.h>
#include <Zmey/Graphics/Backend/BackendDeclarations.h>
#include <cstdint>

namespace Zmey
{
namespace Graphics
{

struct FrameData;

class RendererInterface
{
public:
	RendererInterface();

	bool CreateWindowSurface(WindowHandle handle);

	void RenderFrame(FrameData& frameData);

	bool CheckIfFrameCompleted(uint64_t frameIndex);
private:
	void PrepareData(FrameData& frameData);
	void GenerateCommands(FrameData& frameData, uint32_t imageIndex);
	void Present(FrameData& frameData, uint32_t imageIndex);


	stl::unique_ptr<Backend::Backend> m_Backend;
	// TODO: atomic
	uint64_t LastCompletedFrame = 0;

	Backend::Semaphore* m_ImageAvailableSemaphore;
	Backend::Semaphore* m_RenderFinishedSemaphore;
	Backend::RenderPass* m_MainRenderPass;
	Backend::PipelineState* m_RectsPipelineState;
	stl::vector<Backend::Framebuffer*> m_SwapChainFramebuffers;
	stl::vector<Backend::CommandList*> m_CommandLists;
};

}
}
