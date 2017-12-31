#include <Zmey/Logging.h>
#include <Zmey/Memory/MemoryManagement.h> // For TempAllocator
#include <Zmey/Graphics/FrameData.h>
#include <Zmey/Graphics/Renderer.h>
#include <Zmey/Graphics/RendererGlobals.h>

#include <Zmey/Graphics/Features.h>

#include <Zmey/Graphics/Backend/Device.h>
#include <Zmey/Graphics/Backend/CommandList.h>

#include <Zmey/ResourceLoader/DDSLoader.h>


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

bool RendererInterface::CreateWindowSurface(WindowHandle handle)
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
	desc.Layout.Elements.push_back(Backend::InputElement{ "NORMAL", 0, Backend::InputElementFormat::Float3, 0, 3 * sizeof(float)});
	desc.Topology = Backend::PrimitiveTopology::TriangleList;
	m_Data.MeshesPipelineState = m_Device->CreateGraphicsPipelineState(desc);

	auto swapChainCount = m_Device->GetSwapChainBuffers();
	m_SwapChainFramebuffers.reserve(swapChainCount);
	m_CommandLists.reserve(swapChainCount);
	for (auto i = 0u; i < swapChainCount; ++i)
	{
		m_SwapChainFramebuffers.push_back(m_Device->CreateFramebuffer(m_Device->GetSwapChainImageView(i)));
		m_CommandLists.push_back(m_Device->CreateCommandList());
	}

	return true;
}

void RendererInterface::Unitialize()
{
	m_Data.BufferManager.DestroyResources();

	for (auto& list : m_CommandLists)
	{
		m_Device->DestroyCommandList(list);
	}

	m_Device->DestroyGraphicsPipelineState(m_Data.MeshesPipelineState);
	for (auto& rtv : m_SwapChainFramebuffers)
	{
		m_Device->DestroyFramebuffer(rtv);
	}

	m_Device.reset();
}

void RendererInterface::UploadTextures()
{
	if (m_TextureToUpload.empty())
	{
		return;
	}

	auto uploadList = m_Device->CreateCommandList();
	uploadList->BeginRecording();
	for (const auto& texData : m_TextureToUpload)
	{
		//TODO: find way not to initialize new loader
		DDSLoader loader(texData.Data.data(), texData.Data.size());

		auto imageDataSize = loader.GetHeight() * loader.GetWidth() * 4;// TODO: Bytes per pixel for format
		m_Data.UploadHeap.CopyDataToTexture(uploadList, imageDataSize, loader.GetImageData(), texData.Texture);
	}

	uploadList->EndRecording();
	m_Device->SubmitCommandList(uploadList);
	// TODO: maybe wait for execution
	m_Device->DestroyCommandList(uploadList);

	m_TextureToUpload.clear();
}

void RendererInterface::GatherData(FrameData& frameData, World& world)
{
	for (auto gatherData : m_Features.GatherDataPtrs)
	{
		gatherData(frameData, world);
	}
}

void RendererInterface::PrepareData(FrameData& frameData)
{
	UploadTextures();

	for (auto prepareData : m_Features.PrepareDataPtrs)
	{
		prepareData(frameData, m_Data);
	}
}

void RendererInterface::GenerateCommands(FrameData& frameData, uint32_t imageIndex)
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

	m_CommandLists[imageIndex]->EndRenderPass(m_SwapChainFramebuffers[imageIndex]);
	// End Main pass on PlayerView

	m_CommandLists[imageIndex]->EndRecording();

	m_Device->SubmitCommandList(m_CommandLists[imageIndex]);
}

void RendererInterface::Present(FrameData& frameData, uint32_t imageIndex)
{
	m_Device->Present(imageIndex);
}

void RendererInterface::RenderFrame(FrameData& frameData)
{
	uint32_t imageIndex = m_Device->AcquireNextSwapChainImage();

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

MeshHandle RendererInterface::MeshLoaded(stl::vector<uint8_t>&& data)
{
	if (data.size() < sizeof(MeshDataHeader))
	{
		return MeshHandle(-1);
	}

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

TextureHandle RendererInterface::TextureLoaded(stl::vector<uint8_t>&& data)
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
	upload.Data = std::move(data);
	upload.Texture = m_Data.TextureManager.GetTexture(result);
	m_TextureToUpload.push_back(std::move(upload));

	return result;
}
}
}