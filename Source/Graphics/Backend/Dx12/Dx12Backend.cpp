#include <Zmey/Graphics/Backend/Dx12/Dx12Backend.h>
#include <Zmey/Logging.h>

#include <Zmey/Graphics/Backend/Dx12/Dx12Shaders.h>

#include <Zmey/Graphics/Backend/Dx12/Dx12CommandList.h>

namespace Zmey
{
namespace Graphics
{
namespace Backend
{

namespace
{
	inline DXGI_FORMAT InputElementFormatToDx12(InputElementFormat format)
	{
		switch (format)
		{
		case Zmey::Graphics::Backend::InputElementFormat::Float3:
			return DXGI_FORMAT_R32G32B32_FLOAT;
		default:
			assert(false);
			break;
		}

		return DXGI_FORMAT_UNKNOWN;
	}
}

Dx12Backend::Dx12Backend()
{
	UINT dxgiFactoryFlags = 0;

#if defined(_DEBUG)
	{
		ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();

			// Enable additional debug layers.
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		}
	}
#endif
	CHECK_SUCCESS(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&m_Factory)));

	ComPtr<IDXGIAdapter1> adapter;

	for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != m_Factory->EnumAdapters1(adapterIndex, &adapter); ++adapterIndex)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			continue;
		}

		if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_Device))))
		{
			break;
		}
	}

	if (!m_Device)
	{
		LOG(Fatal, Dx12, "No valid adapter for creating dx12 device");
		return;
	}

	// Describe and create the command queue.
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	CHECK_SUCCESS(m_Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_GraphicsQueue)));
}


void Dx12Backend::Initialize(WindowHandle windowHandle)
{
	auto scope = TempAllocator::GetTlsAllocator().ScopeNow();

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = 2;
	swapChainDesc.Width = 1280;
	swapChainDesc.Height = 720;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;

	ComPtr<IDXGISwapChain1> swapChain;
	CHECK_SUCCESS(m_Factory->CreateSwapChainForHwnd(
		m_GraphicsQueue.Get(),
		HWND(windowHandle),
		&swapChainDesc,
		nullptr,
		nullptr,
		&swapChain
	));

	swapChain.As(&m_SwapChain);

	CHECK_SUCCESS(m_Factory->MakeWindowAssociation(HWND(windowHandle), DXGI_MWA_NO_ALT_ENTER));

	UINT rtvDescriptorSize;
	// Create descriptor heaps.
	{
		// Describe and create a render target view (RTV) descriptor heap.
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = 2;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		CHECK_SUCCESS(m_Device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_RTVHeap)));

		rtvDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}

	// Create frame resources.
	{
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RTVHeap->GetCPUDescriptorHandleForHeapStart());

		// Create a RTV for each frame.
		for (UINT n = 0; n < 2; n++)
		{
			m_SwapChainImages.push_back(SwapChainImage{});
			CHECK_SUCCESS(m_SwapChain->GetBuffer(n, IID_PPV_ARGS(&m_SwapChainImages[n].Resource)));
			m_Device->CreateRenderTargetView(m_SwapChainImages[n].Resource.Get(), nullptr, rtvHandle);
			m_SwapChainImages[n].RTVHandle = rtvHandle;

			rtvHandle.ptr += 1 * rtvDescriptorSize;
		}
	}

	CHECK_SUCCESS(m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_CommandAllocator)));

	// Create synchronization objects.
	{
		m_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence));
		m_FenceValue = 1;

		// Create an event handle to use for frame synchronization.
		m_FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	}
}

Shader* Dx12Backend::CreateShader()
{
	return nullptr;
}

void Dx12Backend::DestroyShader(Shader* shader)
{

}

PipelineState* Dx12Backend::CreatePipelineState(const PipelineStateDesc& psDesc)
{
	TEMP_ALLOCATOR_SCOPE;

	D3D12_ROOT_PARAMETER rootParam;
	rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	rootParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParam.Constants.Num32BitValues = 8;
	rootParam.Constants.RegisterSpace = 0;
	rootParam.Constants.ShaderRegister = 0;

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rootSignatureDesc.NumParameters = 1;
	rootSignatureDesc.pParameters = &rootParam;
	rootSignatureDesc.NumStaticSamplers = 0;
	rootSignatureDesc.pStaticSamplers = nullptr;

	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> error;
	ID3D12RootSignature* rootSignature;
	CHECK_SUCCESS(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));

	CHECK_SUCCESS(m_Device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature)));

	D3D12_GRAPHICS_PIPELINE_STATE_DESC desc;
	ZeroMemory(&desc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

	tmp::vector<D3D12_INPUT_ELEMENT_DESC> inputElements;
	inputElements.reserve(psDesc.Layout.Elements.size());
	for (auto& ie : psDesc.Layout.Elements)
	{
		D3D12_INPUT_ELEMENT_DESC ieDesc;
		ieDesc.SemanticName = ie.Semantic;
		ieDesc.SemanticIndex = ie.SemanticIndex;
		ieDesc.Format = InputElementFormatToDx12(ie.Format);
		ieDesc.InputSlot = ie.Slot;
		ieDesc.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
		ieDesc.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
		ieDesc.InstanceDataStepRate = 0;
		inputElements.push_back(ieDesc);
	}

	desc.InputLayout.pInputElementDescs = inputElements.data();
	desc.InputLayout.NumElements = UINT(inputElements.size());

	desc.pRootSignature = rootSignature;

	desc.VS.pShaderBytecode = psDesc.VertexShader.Data;
	desc.VS.BytecodeLength = psDesc.VertexShader.Size;
	desc.PS.pShaderBytecode = psDesc.PixelShader.Data;
	desc.PS.BytecodeLength = psDesc.PixelShader.Size;

	desc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	desc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	desc.RasterizerState.FrontCounterClockwise = TRUE;
	desc.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	desc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	desc.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	desc.RasterizerState.DepthClipEnable = TRUE;
	desc.RasterizerState.MultisampleEnable = FALSE;
	desc.RasterizerState.AntialiasedLineEnable = FALSE;
	desc.RasterizerState.ForcedSampleCount = 0;
	desc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	desc.BlendState.AlphaToCoverageEnable = FALSE;
	desc.BlendState.IndependentBlendEnable = FALSE;
	const D3D12_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc =
	{
		FALSE,FALSE,
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_LOGIC_OP_NOOP,
		D3D12_COLOR_WRITE_ENABLE_ALL,
	};
	for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
		desc.BlendState.RenderTarget[i] = defaultRenderTargetBlendDesc;

	desc.DepthStencilState.DepthEnable = FALSE;
	desc.DepthStencilState.StencilEnable = FALSE;

	desc.SampleMask = UINT_MAX;
	desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	desc.NumRenderTargets = 1;
	desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;

	ID3D12PipelineState* state;
	CHECK_SUCCESS(m_Device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&state)));

	auto result = new Dx12PipelineState;
	result->PipelineState = state;
	result->RootSignature = rootSignature;

	return result;
}

void Dx12Backend::DestroyPipelineState(PipelineState* state)
{
	auto dx12state = reinterpret_cast<Dx12PipelineState*>(state);
	dx12state->PipelineState->Release();
	dx12state->RootSignature->Release();
	delete state;
}

ImageView* Dx12Backend::CreateImageView()
{
	return nullptr;
}

void Dx12Backend::DestroyImageView(ImageView* imageView)
{

}

Buffer* Dx12Backend::CreateBuffer(uint32_t size)
{
	D3D12_HEAP_PROPERTIES heapProperties;
	heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProperties.CreationNodeMask = 0;
	heapProperties.VisibleNodeMask = 0;

	D3D12_RESOURCE_DESC desc;
	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	desc.Alignment = 0;
	desc.Width = size;
	desc.Height = 1;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	desc.Flags = D3D12_RESOURCE_FLAG_NONE;

	ID3D12Resource* buffer;
	CHECK_SUCCESS(m_Device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&buffer)
	));

	auto result = new Dx12Buffer;
	result->Buffer = buffer;
	result->State = D3D12_RESOURCE_STATE_GENERIC_READ;
	result->Size = size;
	return result;
}

void Dx12Backend::DestroyBuffer(Buffer* buffer)
{
	reinterpret_cast<Dx12Buffer*>(buffer)->Buffer->Release();
	delete buffer;
}

void* Dx12Buffer::Map()
{
	assert(State == D3D12_RESOURCE_STATE_GENERIC_READ);
	void* mappedMemory = nullptr;
	CHECK_SUCCESS(Buffer->Map(0, nullptr, &mappedMemory));
	return mappedMemory;
}

void Dx12Buffer::Unmap()
{
	Buffer->Unmap(0, nullptr);
}

uint32_t Dx12Backend::GetSwapChainBuffers()
{
	return uint32_t(m_SwapChainImages.size());
}

ImageView* Dx12Backend::GetSwapChainImageView(uint32_t index)
{
	return reinterpret_cast<ImageView*>(&m_SwapChainImages[index]);
}

uint32_t Dx12Backend::AcquireNextSwapChainImage()
{
	return m_SwapChain->GetCurrentBackBufferIndex();
}

void Dx12Backend::Present(uint32_t imageIndex)
{
	m_SwapChain->Present(1, 0);

	const UINT64 fence = m_FenceValue;
	m_GraphicsQueue->Signal(m_Fence.Get(), fence);
	m_FenceValue++;

	// Wait until the previous frame is finished.
	if (m_Fence->GetCompletedValue() < fence)
	{
		m_Fence->SetEventOnCompletion(fence, m_FenceEvent);
		WaitForSingleObject(m_FenceEvent, INFINITE);
	}

	m_CommandAllocator->Reset();
}

Framebuffer* Dx12Backend::CreateFramebuffer(ImageView* imageView)
{
	auto view = reinterpret_cast<SwapChainImage*>(imageView);
	auto result = new Dx12Framebuffer;
	result->TextureResource = view->Resource.Get();
	result->RTV = view->RTVHandle;
	return result;
}

void Dx12Backend::DestroyFramebuffer(Framebuffer* framebuffer)
{
	delete framebuffer;
}

CommandList* Dx12Backend::CreateCommandList()
{
	ID3D12GraphicsCommandList* list;
	m_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocator.Get(), nullptr, IID_PPV_ARGS(&list));

	list->Close();

	auto result = new Dx12CommandList;
	result->CmdList = list;
	result->CmdAllocator = m_CommandAllocator.Get();

	return result;
}

void Dx12Backend::DestroyCommandList(CommandList* list)
{
	reinterpret_cast<Dx12CommandList*>(list)->CmdList->Release();
	delete list;
}

void Dx12Backend::SubmitCommandList(CommandList* list)
{
	ID3D12CommandList* l = reinterpret_cast<Dx12CommandList*>(list)->CmdList;
	m_GraphicsQueue->ExecuteCommandLists(1, &l);
}

// TODO(alex): extract to non Dx12 header
Backend* CreateBackend()
{
	return new Dx12Backend;
}

}
}
}