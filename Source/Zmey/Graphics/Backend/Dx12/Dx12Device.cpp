#include <Zmey/Graphics/Backend/Dx12/Dx12Device.h>

#ifdef USE_DX12

#pragma comment(lib, "d3d12.lib")

#include <Zmey/Logging.h>

#include <Zmey/Graphics/Backend/Dx12/Dx12Shaders.h>
#include <Zmey/Graphics/Backend/Dx12/Dx12CommandList.h>
#include <Zmey/Graphics/Backend/Dx12/Dx12Texture.h>
#include <Zmey/Graphics/Backend/BackendResourceSet.h>

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
	case InputElementFormat::RGBA8:
		return DXGI_FORMAT_R8G8B8A8_UNORM;
	case InputElementFormat::Float2:
		return DXGI_FORMAT_R32G32_FLOAT;
	case InputElementFormat::Float3:
		return DXGI_FORMAT_R32G32B32_FLOAT;
	case InputElementFormat::Float4:
		return DXGI_FORMAT_R32G32B32A32_FLOAT;
	default:
		NOT_REACHED();
		break;
	}

	return DXGI_FORMAT_UNKNOWN;
}
}

Dx12Device::Dx12Device()
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


void Dx12Device::Initialize(WindowHandle windowHandle)
{
	auto scope = TempAllocator::GetTlsAllocator().ScopeNow();

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = 2;
	swapChainDesc.Width = 0;
	swapChainDesc.Height = 0;
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

	{
		DXGI_SWAP_CHAIN_DESC1 realDesc;
		m_SwapChain->GetDesc1(&realDesc);
		m_SwapChainSize = Vector2(realDesc.Width, realDesc.Height);
	}

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

		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
		dsvHeapDesc.NumDescriptors = 1;
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		CHECK_SUCCESS(m_Device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_DSVHeap)));
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

		// Create DepthStencil texture
		{
			D3D12_HEAP_PROPERTIES heapProperties;
			heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
			heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
			heapProperties.CreationNodeMask = 0;
			heapProperties.VisibleNodeMask = 0;

			D3D12_RESOURCE_DESC desc;
			desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			desc.Alignment = 0;
			desc.Width = m_SwapChainSize.x;
			desc.Height = m_SwapChainSize.y;
			desc.DepthOrArraySize = 1;
			desc.MipLevels = 1;
			desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
			desc.SampleDesc.Count = 1;
			desc.SampleDesc.Quality = 0;
			desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

			D3D12_CLEAR_VALUE clearValue;
			clearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
			clearValue.DepthStencil.Depth = 1.0f;
			clearValue.DepthStencil.Stencil = 0;

			CHECK_SUCCESS(m_Device->CreateCommittedResource(
				&heapProperties,
				D3D12_HEAP_FLAG_NONE,
				&desc,
				D3D12_RESOURCE_STATE_DEPTH_WRITE,
				&clearValue,
				IID_PPV_ARGS(&m_DSV)
			));


			m_DSVHandle = m_DSVHeap->GetCPUDescriptorHandleForHeapStart();
			m_Device->CreateDepthStencilView(m_DSV.Get(), nullptr, m_DSVHandle);
		}
	}

	// Create synchronization objects.
	{
		m_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence));
		m_FenceValue = 1;

		// Create an event handle to use for frame synchronization.
		m_FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	}
}

namespace
{
D3D12_CULL_MODE CullModeToDx12(CullMode mode)
{
	switch (mode)
	{
	case CullMode::None:
		return D3D12_CULL_MODE_NONE;
	case CullMode::Front:
		return D3D12_CULL_MODE_FRONT;
	case CullMode::Back:
		return D3D12_CULL_MODE_BACK;
	default:
		NOT_REACHED();
		return D3D12_CULL_MODE_NONE;
	}
}
}

GraphicsPipelineState* Dx12Device::CreateGraphicsPipelineState(const GraphicsPipelineStateDesc& psDesc)
{
	TEMP_ALLOCATOR_SCOPE;

	D3D12_ROOT_PARAMETER rootParam;
	rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	rootParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParam.Constants.Num32BitValues = GatherPushConstantCount(psDesc.ResourceTable.ResourceSets);
	rootParam.Constants.RegisterSpace = 0;
	rootParam.Constants.ShaderRegister = 0;

	D3D12_DESCRIPTOR_RANGE range;
	range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	range.NumDescriptors = 1;
	range.BaseShaderRegister = 0;
	range.RegisterSpace = 0;
	range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER rootParam2;
	rootParam2.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam2.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParam2.DescriptorTable.NumDescriptorRanges = 1;
	rootParam2.DescriptorTable.pDescriptorRanges = &range;

	D3D12_STATIC_SAMPLER_DESC sampler;
	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.MipLODBias = 0;
	sampler.MaxAnisotropy = 0;
	sampler.MinLOD = 0;
	sampler.MaxLOD = 0;
	sampler.ShaderRegister = 0;
	sampler.RegisterSpace = 0;
	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_ROOT_PARAMETER params[2] = { rootParam, rootParam2 };
	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rootSignatureDesc.NumParameters = 2;
	rootSignatureDesc.pParameters = params;
	rootSignatureDesc.NumStaticSamplers = 1;
	rootSignatureDesc.pStaticSamplers = &sampler;

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
		ieDesc.AlignedByteOffset = ie.Offset;
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
	desc.RasterizerState.CullMode = CullModeToDx12(psDesc.Rasterizer.CullMode);
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
		psDesc.Blend.BlendEnable ? TRUE : FALSE,FALSE,
		D3D12_BLEND_SRC_ALPHA, D3D12_BLEND_INV_SRC_ALPHA, D3D12_BLEND_OP_ADD,
		D3D12_BLEND_INV_SRC_ALPHA, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_LOGIC_OP_NOOP,
		D3D12_COLOR_WRITE_ENABLE_ALL,
	};
	for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
		desc.BlendState.RenderTarget[i] = defaultRenderTargetBlendDesc;

	desc.DepthStencilState.DepthEnable = psDesc.DepthStencil.DepthEnable ? TRUE : FALSE;
	desc.DepthStencilState.StencilEnable = FALSE;
	desc.DepthStencilState.DepthWriteMask = psDesc.DepthStencil.DepthWrite ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
	desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;

	desc.SampleMask = UINT_MAX;
	desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; // TODO: get this from desc
	desc.NumRenderTargets = 1;
	desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	desc.SampleDesc.Count = 1;

	ID3D12PipelineState* state;
	CHECK_SUCCESS(m_Device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&state)));

	auto result = new Dx12GraphicsPipelineState;
	result->Desc = psDesc;
	result->PipelineState = state;
	result->RootSignature = rootSignature;

	return result;
}

void Dx12Device::DestroyGraphicsPipelineState(GraphicsPipelineState* state)
{
	auto dx12state = reinterpret_cast<Dx12GraphicsPipelineState*>(state);
	dx12state->PipelineState->Release();
	dx12state->RootSignature->Release();
	delete state;
}

Buffer* Dx12Device::CreateBuffer(uint32_t size, BufferUsage usage)
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

void Dx12Device::DestroyBuffer(Buffer* buffer)
{
	reinterpret_cast<Dx12Buffer*>(buffer)->Buffer->Release();
	delete buffer;
}

Texture* Dx12Device::CreateTexture(uint32_t width, uint32_t height, PixelFormat format)
{
	D3D12_HEAP_PROPERTIES heapProperties;
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProperties.CreationNodeMask = 0;
	heapProperties.VisibleNodeMask = 0;

	D3D12_RESOURCE_DESC desc;
	desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	desc.Alignment = 0;
	desc.Width = width;
	desc.Height = height;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.Format = PixelFormatToDx12(format);
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	desc.Flags = D3D12_RESOURCE_FLAG_NONE;

	ID3D12Resource* texture;
	CHECK_SUCCESS(m_Device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&texture)
	));

	auto result = new Dx12Texture;
	result->Texture = texture;
	result->State = D3D12_RESOURCE_STATE_GENERIC_READ;
	result->Width = width;
	result->Height = height;
	result->Format = format;
	return result;
}

void Dx12Device::DestroyTexture(Texture* texture)
{
	delete texture;
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

uint32_t Dx12Device::GetSwapChainBuffers()
{
	return uint32_t(m_SwapChainImages.size());
}

ImageView* Dx12Device::GetSwapChainImageView(uint32_t index)
{
	return reinterpret_cast<ImageView*>(&m_SwapChainImages[index]);
}

uint32_t Dx12Device::AcquireNextSwapChainImage()
{
	return m_SwapChain->GetCurrentBackBufferIndex();
}

void Dx12Device::Present(uint32_t imageIndex)
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
}

Framebuffer* Dx12Device::CreateFramebuffer(ImageView* imageView)
{
	auto view = reinterpret_cast<SwapChainImage*>(imageView);
	auto result = new Dx12Framebuffer;
	result->TextureResource = view->Resource.Get();
	result->RTV = view->RTVHandle;
	result->DSResource = m_DSV.Get();
	result->DSV = m_DSVHandle;
	result->Width = m_SwapChainSize.x;
	result->Height = m_SwapChainSize.y;
	return result;
}

void Dx12Device::DestroyFramebuffer(Framebuffer* framebuffer)
{
	delete framebuffer;
}

CommandList* Dx12Device::CreateCommandList()
{
	ID3D12CommandAllocator* allocator;
	CHECK_SUCCESS(m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator)));

	ID3D12GraphicsCommandList* list;
	CHECK_SUCCESS(m_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocator, nullptr, IID_PPV_ARGS(&list)));

	list->Close();

	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = 1024; // TODO: This is totally random
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ID3D12DescriptorHeap* srvHeap;
	CHECK_SUCCESS(m_Device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&srvHeap)));

	auto result = new Dx12CommandList;
	result->CmdList = list;
	result->CmdAllocator = allocator;
	result->SRVHeap = srvHeap;

	return result;
}

void Dx12Device::DestroyCommandList(CommandList* list)
{
	delete list;
}

void Dx12Device::SubmitCommandLists(CommandList** lists, uint32_t count)
{
	TEMP_ALLOCATOR_SCOPE;
	tmp::vector<ID3D12CommandList*> cmdLists;
	for (auto i = 0u; i < count; ++i)
	{
		cmdLists.push_back(reinterpret_cast<Dx12CommandList*>(lists[i])->CmdList.Get());
	}
	m_GraphicsQueue->ExecuteCommandLists(count, cmdLists.data());
}

// TODO(alex): extract to non Dx12 header
Device* CreateBackendDevice()
{
	return new Dx12Device;
}

}
}
}

#endif