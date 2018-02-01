#pragma once

#include <Zmey/Platform/Platform.h>
#include <Zmey/Memory/MemoryManagement.h>
#include <Zmey/Graphics/Backend/BackendDeclarations.h>
#include <Zmey/Graphics/GraphicsObjects.h>
#include <Zmey/Graphics/RenderPasses.h>
#include <Zmey/Graphics/View.h>

#include <Zmey/Graphics/Managers/BufferManager.h>
#include <Zmey/Graphics/Managers/TextureManager.h>
#include <Zmey/Graphics/Managers/MeshManager.h>
#include <Zmey/Graphics/Managers/MaterialManager.h>
#include <Zmey/Graphics/Managers/UploadHeap.h>
#include <stdint.h>

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
		, TextureManager(device)
	{}

	BufferManager BufferManager;
	TextureManager TextureManager;
	MaterialManager MaterialManager;
	MeshManager MeshManager;
	UploadHeap UploadHeap;

	// TODO(alex): Add PipelineState Manager and remove this
	Backend::GraphicsPipelineState* MeshesPipelineState;

	struct UIData
	{
		Backend::GraphicsPipelineState* PipelineState = nullptr;
		Backend::Buffer* VertexBuffers[2] = { nullptr, nullptr };
		Backend::Buffer* IndexBuffers[2] = { nullptr, nullptr };
	};

	UIData UIData;
};

class Renderer
{
public:
	Renderer();
	~Renderer();

	bool CreateWindowSurface(WindowHandle handle);

	void RenderFrame(FrameData& frameData);
	void GatherData(FrameData& frameData, World& world);

	bool CheckIfFrameCompleted(uint64_t frameIndex);

	UVector2 GetSwapChainSize();

	// TODO: This is very weird to be here.
	MeshHandle MeshLoaded(stl::vector<uint8_t>&& data);
	MaterialHandle MaterialLoaded(stl::vector<uint8_t>&& data);
	TextureHandle TextureLoaded(const uint8_t* data, uint64_t size);
	TextureHandle UITextureLoaded(uint8_t* data, uint32_t width, uint32_t height);

	RendererData& GetRendererData() { return m_Data; }

private:
	void PrepareData(FrameData& frameData);
	void GenerateCommands(FrameData& frameData, uint32_t imageIndex);
	void Present(FrameData& frameData, uint32_t imageIndex);

	void UploadTextures();

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

	struct TextureDataToUpload
	{
		stl::vector<uint8_t> Data;
		Backend::Texture* Texture;
		uint64_t StartOffsetInData;
		uint32_t ActualDataSize;
	};
	stl::vector<TextureDataToUpload> m_TextureToUpload;
};

}
}
