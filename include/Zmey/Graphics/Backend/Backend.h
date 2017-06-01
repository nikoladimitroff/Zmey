#pragma once

#include <stdint.h>
#include <Zmey/Graphics/Backend/BackendDeclarations.h>
#include <Zmey/Memory/MemoryManagement.h>

#include <Zmey/Platform/Platform.h>

namespace Zmey
{
namespace Graphics
{
namespace Backend
{

struct Shader
{
	const unsigned char* Data;
	size_t Size;
};

enum class InputElementFormat : uint8_t
{
	Float3
};

struct InputElement
{
	const char* Semantic;
	uint8_t SemanticIndex;
	InputElementFormat Format;
	uint8_t Slot;
	uint32_t Offset;
};

struct InputLayout
{
	stl::vector<InputElement> Elements;
};

struct PipelineStateDesc
{
	Shader VertexShader;
	Shader PixelShader;
	InputLayout Layout;
};

class PipelineState
{};

class Framebuffer
{};

class ImageView
{};

enum class BufferUsage
{
	Vertex,
	Index
};

class Buffer
{
public:
	virtual void* Map() = 0;
	virtual void Unmap() = 0;

	uint32_t Size;
};

class Backend
{
public:
	virtual ~Backend() {}

	virtual void Initialize(WindowHandle windowHandle) = 0;

	virtual Shader* CreateShader() = 0;
	virtual void DestroyShader(Shader* shader) = 0;

	virtual PipelineState* CreatePipelineState(const PipelineStateDesc& desc) = 0;
	virtual void DestroyPipelineState(PipelineState* state) = 0;

	virtual CommandList* CreateCommandList() = 0;
	virtual void DestroyCommandList(CommandList* list) = 0;
	virtual void SubmitCommandList(CommandList* list) = 0;

	virtual Framebuffer* CreateFramebuffer(ImageView* imageView) = 0;
	virtual void DestroyFramebuffer(Framebuffer* framebuffer) = 0;

	virtual ImageView* CreateImageView() = 0;
	virtual void DestroyImageView(ImageView* imageView) = 0;

	virtual Buffer* CreateBuffer(uint32_t size) = 0;
	virtual void DestroyBuffer(Buffer* buffer) = 0;

	virtual uint32_t GetSwapChainBuffers() = 0;
	virtual ImageView* GetSwapChainImageView(uint32_t index) = 0;
	virtual uint32_t AcquireNextSwapChainImage() = 0;

	virtual void Present(uint32_t imageIndex) = 0;
};

Backend* CreateBackend();
}
}
}