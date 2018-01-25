#include <Zmey/Logging.h>
#include <Zmey/Memory/MemoryManagement.h> // For TempAllocator
#include <Zmey/Graphics/FrameData.h>
#include <Zmey/Graphics/Renderer.h>
#include <Zmey/Graphics/RendererGlobals.h>

#include <Zmey/Graphics/Features.h>

#include <Zmey/Graphics/Backend/Device.h>
#include <Zmey/Graphics/Backend/CommandList.h>
#include <Zmey/Graphics/Backend/Buffer.h>

#include <Zmey/ResourceLoader/DDSLoader.h>

#include <imgui/imgui.h>

//TODO(alex): remove this
#include <Zmey/Graphics/Backend/Dx12/Dx12Shaders.h>
#include <Zmey/Graphics/Backend/Vulkan/VulkanShaders.h>

namespace Zmey
{
namespace Graphics
{

// Storage for globals
namespace Globals
{
Backend::Device* g_Device;
}

Renderer::~Renderer()
{
	m_Data.BufferManager.DestroyResources();

	for (auto& list : m_CommandLists)
	{
		m_Device->DestroyCommandList(list);
	}

	if (m_Data.MeshesPipelineState)
	{
		m_Device->DestroyGraphicsPipelineState(m_Data.MeshesPipelineState);
	}
	for (auto& rtv : m_SwapChainFramebuffers)
	{
		m_Device->DestroyFramebuffer(rtv);
	}

	m_Device.reset();
}

bool Renderer::CreateWindowSurface(WindowHandle handle)
{
	m_Device->Initialize(handle);

	Backend::GraphicsPipelineStateDesc desc;
#ifdef USE_DX12
	desc.VertexShader = Backend::Shader{ Backend::Shaders::Mesh::g_VertexShaderMain, sizeof(Backend::Shaders::Mesh::g_VertexShaderMain) };
	desc.PixelShader = Backend::Shader{ Backend::Shaders::Mesh::g_PixelShaderMain, sizeof(Backend::Shaders::Mesh::g_PixelShaderMain) };
#else
	desc.VertexShader = Backend::Shader{ (const unsigned char*)Backend::Shaders::g_MeshVS, Backend::Shaders::g_MeshVSSize };
	desc.PixelShader = Backend::Shader{ (const unsigned char*)Backend::Shaders::g_MeshPS, Backend::Shaders::g_MeshPSSize };
#endif
	desc.Layout.Elements.push_back(Backend::InputElement{ "POSITION", 0, Backend::InputElementFormat::Float3, 0, 0 });
	desc.Layout.Elements.push_back(Backend::InputElement{ "NORMAL", 0, Backend::InputElementFormat::Float3, 0, 3 * sizeof(float) });
	desc.Layout.Elements.push_back(Backend::InputElement{ "TEXCOORD", 0, Backend::InputElementFormat::Float2, 0, 6 * sizeof(float)});
	desc.Topology = Backend::PrimitiveTopology::TriangleList;
	desc.ResourceTable.NumPushConstants = 43;

	m_Data.MeshesPipelineState = m_Device->CreateGraphicsPipelineState(desc);

	auto swapChainCount = m_Device->GetSwapChainBuffers();
	m_SwapChainFramebuffers.reserve(swapChainCount);
	m_CommandLists.reserve(swapChainCount);
	for (auto i = 0u; i < swapChainCount; ++i)
	{
		m_SwapChainFramebuffers.push_back(m_Device->CreateFramebuffer(m_Device->GetSwapChainImageView(i)));
		m_CommandLists.push_back(m_Device->CreateCommandList());
	}

	// UI
	{
		Backend::GraphicsPipelineStateDesc uiDesc;
#ifdef USE_DX12
		uiDesc.VertexShader = Backend::Shader{ Backend::Shaders::UI::g_VertexShaderMain, sizeof(Backend::Shaders::UI::g_VertexShaderMain) };
		uiDesc.PixelShader = Backend::Shader{ Backend::Shaders::UI::g_PixelShaderMain, sizeof(Backend::Shaders::UI::g_PixelShaderMain) };
#else
		uiDesc.VertexShader = Backend::Shader{ (const unsigned char*)Backend::Shaders::g_UIVS, Backend::Shaders::g_UIVSSize };
		uiDesc.PixelShader = Backend::Shader{ (const unsigned char*)Backend::Shaders::g_UIPS, Backend::Shaders::g_UIPSSize };
#endif
		uiDesc.Layout.Elements.push_back(Backend::InputElement{ "POSITION", 0, Backend::InputElementFormat::Float2, 0, 0 });
		uiDesc.Layout.Elements.push_back(Backend::InputElement{ "TEXCOORD", 0, Backend::InputElementFormat::Float2, 0, 2 * sizeof(float) });
		uiDesc.Layout.Elements.push_back(Backend::InputElement{ "COLOR", 0, Backend::InputElementFormat::RGBA8, 0, 4 * sizeof(float) });
		uiDesc.Topology = Backend::PrimitiveTopology::TriangleList;
		uiDesc.ResourceTable.NumPushConstants = 4;
		uiDesc.Rasterizer.CullMode = Backend::CullMode::None;
		uiDesc.Blend.BlendEnable = true;
		uiDesc.DepthStencil.DepthEnable = false;
		uiDesc.DepthStencil.DepthWrite = false;

		m_Data.UIData.PipelineState = m_Device->CreateGraphicsPipelineState(uiDesc);
	}

	return true;
}

UVector2 Renderer::GetSwapChainSize()
{
	return m_Device->GetSwapChainSize();
}



void Renderer::UploadTextures()
{
	if (m_TextureToUpload.empty())
	{
		return;
	}

	auto uploadList = m_Device->CreateCommandList();
	uploadList->BeginRecording();
	for (const auto& texData : m_TextureToUpload)
	{
		m_Data.UploadHeap.CopyDataToTexture(
			uploadList,
			texData.ActualDataSize,
			texData.Data.data() + texData.StartOffsetInData,
			texData.Texture);
	}

	uploadList->EndRecording();
	m_Device->SubmitCommandLists(&uploadList, 1);
	// TODO: maybe wait for execution
	m_Device->DestroyCommandList(uploadList);

	m_TextureToUpload.clear();
}

void Renderer::GatherData(FrameData& frameData, World& world)
{
	for (auto gatherData : m_Features.GatherDataPtrs)
	{
		gatherData(frameData, world);
	}
}

void Renderer::PrepareData(FrameData& frameData)
{
	UploadTextures();

	for (auto prepareData : m_Features.PrepareDataPtrs)
	{
		prepareData(frameData, m_Data);
	}
}

void Renderer::GenerateCommands(FrameData& frameData, uint32_t imageIndex)
{
	// TODO: High level render passes

	m_CommandLists[imageIndex]->BeginRecording();

	// Begin Main pass on PlayerView
	m_CommandLists[imageIndex]->BeginRenderPass(m_SwapChainFramebuffers[imageIndex]);

	for (auto generateCommands : m_Features.GenerateCommandsPtrs)
	{
		generateCommands(frameData,
			RenderPass::Main,
			ViewType::PlayerView,
			m_CommandLists[imageIndex],
			m_Data);
	}

	for (auto generateCommands : m_Features.GenerateCommandsPtrs)
	{
		generateCommands(frameData,
			RenderPass::UI,
			ViewType::PlayerView,
			m_CommandLists[imageIndex],
			m_Data);
	}

	m_CommandLists[imageIndex]->EndRenderPass(m_SwapChainFramebuffers[imageIndex]);
	// End Main pass on PlayerView

	m_CommandLists[imageIndex]->EndRecording();

	TEMP_ALLOCATOR_SCOPE;
	tmp::vector<Backend::CommandList*> cmdLists;
	cmdLists.push_back(m_CommandLists[imageIndex]);

	m_Device->SubmitCommandLists(cmdLists.data(), uint32_t(cmdLists.size()));
}

void Renderer::Present(FrameData& frameData, uint32_t imageIndex)
{
	m_Device->Present(imageIndex);
}

void Renderer::RenderFrame(FrameData& frameData)
{
	uint32_t imageIndex = m_Device->AcquireNextSwapChainImage();

	PrepareData(frameData);

	GenerateCommands(frameData, imageIndex);

	Present(frameData, imageIndex);

	LastCompletedFrame = frameData.FrameIndex;
}

bool Renderer::CheckIfFrameCompleted(uint64_t frameIndex)
{
	return frameIndex <= LastCompletedFrame;
}

Renderer::Renderer()
	: m_Device(Backend::CreateBackendDevice())
	, m_Data(m_Device.get())
{
	Globals::g_Device = m_Device.get();

	// Initialize renderer features
	m_Features.GatherDataPtrs.reserve(2);
	m_Features.PrepareDataPtrs.reserve(2);
	m_Features.GenerateCommandsPtrs.reserve(2);

#define REGISTER_FEATURE(NAME, HAS_GATHER, HAS_PREPARE, HAS_GENERATE) \
	if (HAS_GATHER) m_Features.GatherDataPtrs.push_back(Features::NAME::GatherData); \
	if (HAS_PREPARE) m_Features.PrepareDataPtrs.push_back(Features::NAME::PrepareData); \
	if (HAS_GENERATE) m_Features.GenerateCommandsPtrs.push_back(Features::NAME::GenerateCommands);

	RENDER_FEATURE_MACRO_ITERATOR(REGISTER_FEATURE)

#undef REGISTER_FEATURE
}

MeshHandle Renderer::MeshLoaded(stl::vector<uint8_t>&& data)
{
	if (data.size() < sizeof(MeshDataHeader))
	{
		return MeshHandle(-1);
	}
	// TODO: move this code inside the mesh manager
	MeshDataHeader* header = reinterpret_cast<MeshDataHeader*>(data.data());

	Mesh newMesh;
	newMesh.IndexCount = uint32_t(header->IndicesCount);
	newMesh.VertexBuffer = m_Data.BufferManager.CreateStaticBuffer(
		Backend::BufferUsage::Vertex,
		uint32_t(header->VerticesCount * sizeof(MeshVertex)),
		data.data() + sizeof(MeshDataHeader)
	);
	newMesh.IndexBuffer = m_Data.BufferManager.CreateStaticBuffer(
		Backend::BufferUsage::Index,
		uint32_t(header->IndicesCount * sizeof(uint32_t)),
		data.data() + sizeof(MeshDataHeader) + (header->VerticesCount * sizeof(MeshVertex))
	);
	return m_Data.MeshManager.CreateMesh(newMesh);
}

MaterialHandle Renderer::MaterialLoaded(stl::vector<uint8_t>&& data)
{
	auto result = m_Data.MaterialManager.CreateMaterial(*reinterpret_cast<MaterialDataHeader*>(data.data()));
	return result;
}

TextureHandle Renderer::TextureLoaded(stl::vector<uint8_t>&& data)
{
	DDSLoader loader(data.data(), data.size());

	if (!loader.IsValid())
	{
		return TextureHandle(-1);
	}

	auto format = loader.GetPixelFormat();
	if (format == PixelFormat::Unknown)
	{
		return TextureHandle(-1);
	}

	auto result = m_Data.TextureManager.CreateTexture(
		loader.GetWidth(),
		loader.GetHeight(),
		format);

	TextureDataToUpload upload;
	upload.Texture = m_Data.TextureManager.GetTexture(result);
	upload.ActualDataSize = loader.GetWidth() * loader.GetHeight() * 4;//TODO: Take image format into account
	upload.StartOffsetInData = loader.GetImageData() - data.data();
	upload.Data = std::move(data);
	m_TextureToUpload.push_back(std::move(upload));

	return result;
}

TextureHandle Renderer::UITextureLoaded(uint8_t* data, uint32_t width, uint32_t height)
{
	auto result = m_Data.TextureManager.CreateTexture(width, height, PixelFormat::B8G8R8A8);

	TextureDataToUpload upload;
	upload.Data.assign(data, data + (width * height * 4));
	upload.ActualDataSize = width * height * 4;
	upload.StartOffsetInData = 0u;
	upload.Texture = m_Data.TextureManager.GetTexture(result);
	m_TextureToUpload.push_back(std::move(upload));

	return result;
}
}
}