#pragma once

#include <stdint.h>
#include <Zmey/Memory/MemoryManagement.h>

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
	RGBA8,
	Float2,
	Float3,
	Float4
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

enum class PrimitiveTopology : uint8_t
{
	TriangleList
};

struct ResourceTableDesc
{
	uint32_t NumPushConstants = 0;
};

enum class CullMode : uint8_t
{
	None,
	Front,
	Back
};

struct RasterizerState
{
	CullMode CullMode = CullMode::Back;
};

struct BlendState
{
	bool BlendEnable = false;
};

struct DepthStencilState
{
	bool DepthEnable = true;
	bool DepthWrite = true;
};

struct GraphicsPipelineStateDesc
{
	Shader VertexShader;
	Shader PixelShader;
	InputLayout Layout;
	PrimitiveTopology Topology;
	ResourceTableDesc ResourceTable;
	RasterizerState Rasterizer;
	BlendState Blend;
	DepthStencilState DepthStencil;
};

struct GraphicsPipelineState
{
	GraphicsPipelineStateDesc Desc;
};

}
}
}