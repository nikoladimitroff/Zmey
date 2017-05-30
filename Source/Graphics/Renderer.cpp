#include <Zmey/Logging.h>
#include <Zmey/Memory/MemoryManagement.h> // For TempAllocator
#include <Zmey/Graphics/FrameData.h>
#include <Zmey/Graphics/Renderer.h>

#include <Zmey/Graphics/Features.h>

#include <Zmey/Graphics/Backend/Backend.h>
#include <Zmey/Graphics/Backend/CommandList.h>

#include <assimp/scene.h>

//TODO(alex): remove this
#include <Zmey/Graphics/Backend/Dx12/Dx12Shaders.h>

namespace Zmey
{
namespace Graphics
{

bool RendererInterface::CreateWindowSurface(WindowHandle handle)
{
	m_Backend->Initialize(handle);

	Backend::PipelineStateDesc desc;
	desc.VertexShader = Backend::Shader{ Backend::Shaders::Rects::g_VertexShaderMain, sizeof(Backend::Shaders::Rects::g_VertexShaderMain) };
	desc.PixelShader = Backend::Shader{ Backend::Shaders::Rects::g_PixelShaderMain, sizeof(Backend::Shaders::Rects::g_PixelShaderMain) };

	m_Data.RectsPipelineState = m_Backend->CreatePipelineState(desc);

	desc.VertexShader = Backend::Shader{ Backend::Shaders::Mesh::g_VertexShaderMain, sizeof(Backend::Shaders::Mesh::g_VertexShaderMain) };
	desc.PixelShader = Backend::Shader{ Backend::Shaders::Mesh::g_PixelShaderMain, sizeof(Backend::Shaders::Mesh::g_PixelShaderMain) };
	desc.Layout.Elements.push_back(Backend::InputElement{ "POSITION", 0, Backend::InputElementFormat::Float3, 0 });
	m_Data.MeshesPipelineState = m_Backend->CreatePipelineState(desc);

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
	m_Data.BufferManager.DestroyResources();

	for (auto& list : m_CommandLists)
	{
		m_Backend->DestroyCommandList(list);
	}

	m_Backend->DestroyPipelineState(m_Data.RectsPipelineState);
	m_Backend->DestroyPipelineState(m_Data.MeshesPipelineState);
	for (auto& rtv : m_SwapChainFramebuffers)
	{
		m_Backend->DestroyFramebuffer(rtv);
	}

	m_Backend.reset();
}

void RendererInterface::PrepareData(FrameData& frameData)
{
	// TODO: Prepare graphics data
	Features::MeshRenderer::PrepareData(frameData, m_Data);
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
		m_CommandLists[imageIndex],
		m_Data);

	Features::RectRenderer::GenerateCommands(frameData,
		RenderPass::Main,
		ViewType::PlayerView,
		m_CommandLists[imageIndex],
		m_Data);

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

MeshHandle RendererInterface::MeshLoaded(const aiScene* scene)
{
	TEMP_ALLOCATOR_SCOPE;
	if (scene->mNumMeshes > 0)
	{
		auto mesh = scene->mMeshes[0];
		tmp::vector<Vector3> vertices;
		vertices.reserve(mesh->mNumVertices);
		for (auto i = 0u; i < mesh->mNumVertices; ++i)
		{
			auto& aiVector = mesh->mVertices[i];
			vertices.push_back(Vector3(aiVector.x, aiVector.y, aiVector.z));
		}

		tmp::vector<uint32_t> indices;
		indices.reserve(mesh->mNumFaces * 3);
		for (auto i = 0u; i < mesh->mNumFaces; ++i)
		{
			auto& face = mesh->mFaces[i];
			assert(face.mNumIndices == 3);
			indices.push_back(face.mIndices[0]);
			indices.push_back(face.mIndices[1]);
			indices.push_back(face.mIndices[2]);
		}

		Mesh newMesh;
		newMesh.IndexCount = uint32_t(indices.size());
		newMesh.VertexBuffer = m_Data.BufferManager.CreateStaticBuffer(uint32_t(vertices.size() * sizeof(Vector3)), vertices.data());
		newMesh.IndexBuffer = m_Data.BufferManager.CreateStaticBuffer(uint32_t(indices.size() * sizeof(uint32_t)), indices.data());
		return m_Data.MeshManager.CreateMesh(newMesh);
	}

	return MeshHandle(-1);
}
}
}