#pragma once

#include <Zmey/Platform/Platform.h>
#include <Zmey/Memory/MemoryManagement.h>
#include <Zmey/Graphics/Backend/BackendDeclarations.h>
#include <Zmey/Graphics/GraphicsObjects.h>

#include <Zmey/Graphics/Managers/BufferManager.h>
#include <Zmey/Graphics/Managers/MeshManager.h>
#include <cstdint>

// TODO(alex): remove me
struct aiScene;

namespace Zmey
{
namespace Graphics
{

struct FrameData;

struct RendererData
{
	RendererData(Backend::Backend* backend)
		: BufferManager(backend)
	{}

	BufferManager BufferManager;
	MeshManager MeshManager;
};

class RendererInterface
{
public:
	RendererInterface();

	bool CreateWindowSurface(WindowHandle handle);
	void Unitialize();

	void RenderFrame(FrameData& frameData);

	bool CheckIfFrameCompleted(uint64_t frameIndex);

	MeshHandle MeshLoaded(const aiScene* mesh);

private:
	void PrepareData(FrameData& frameData);
	void GenerateCommands(FrameData& frameData, uint32_t imageIndex);
	void Present(FrameData& frameData, uint32_t imageIndex);

	stl::unique_ptr<Backend::Backend> m_Backend;
	// TODO: atomic
	uint64_t LastCompletedFrame = 0;

	Backend::PipelineState* m_RectsPipelineState;
	stl::vector<Backend::Framebuffer*> m_SwapChainFramebuffers;
	stl::vector<Backend::CommandList*> m_CommandLists;

	RendererData m_Data;
};

}
}
