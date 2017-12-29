#pragma once

#include <Zmey/Platform/Platform.h>
#include <Zmey/Memory/MemoryManagement.h>
#include <Zmey/Graphics/Backend/BackendDeclarations.h>
#include <Zmey/Graphics/GraphicsObjects.h>
#include <Zmey/Graphics/RenderPasses.h>
#include <Zmey/Graphics/View.h>

#include <Zmey/Graphics/Managers/BufferManager.h>
#include <Zmey/Graphics/Managers/MeshManager.h>
#include <cstdint>

// TODO(alex): remove me
struct aiScene;

namespace Zmey
{

class World;
namespace Graphics
{

struct FrameData;


struct RendererData
{
	RendererData(Backend::Device* device)
		: BufferManager(device)
	{}

	BufferManager BufferManager;
	MeshManager MeshManager;

	// TODO(alex): Add PipelineState Manager and remove this
	Backend::GraphicsPipelineState* MeshesPipelineState;
};

class RendererInterface
{
public:
	RendererInterface();

	bool CreateWindowSurface(WindowHandle handle);
	void Unitialize();

	void RenderFrame(FrameData& frameData);
	void GatherData(FrameData& frameData, World& world);

	bool CheckIfFrameCompleted(uint64_t frameIndex);

	MeshHandle MeshLoaded(stl::vector<uint8_t>&& data);

private:
	void PrepareData(FrameData& frameData);
	void GenerateCommands(FrameData& frameData, uint32_t imageIndex);
	void Present(FrameData& frameData, uint32_t imageIndex);

	stl::unique_ptr<Backend::Device> m_Device;
	// TODO: atomic
	uint64_t LastCompletedFrame = 0;

	stl::vector<Backend::Framebuffer*> m_SwapChainFramebuffers;
	stl::vector<Backend::CommandList*> m_CommandLists;

	struct RenderFeatures
	{
		using GatherDataFunctionPtr = void (*)(FrameData&, World&); //TODO(alex): make this const after there managers can be getted with constnest
		using PrepareDataFunctionPtr = void (*)(FrameData&, RendererData&);
		using GenerateCommandsFunctionPtr = void (*)(FrameData&, RenderPass, ViewType, Backend::CommandList*, RendererData&);

		stl::vector<GatherDataFunctionPtr> GatherDataPtrs;
		stl::vector<PrepareDataFunctionPtr> PrepareDataPtrs;
		stl::vector<GenerateCommandsFunctionPtr> GenerateCommandsPtrs;
	};

	RenderFeatures m_Features;

	RendererData m_Data;
};

}
}
