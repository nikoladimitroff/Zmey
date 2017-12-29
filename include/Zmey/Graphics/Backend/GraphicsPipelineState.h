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

enum class PrimitiveTopology : uint8_t
{
	TriangleList
};

struct GraphicsPipelineStateDesc
{
	Shader VertexShader;
	Shader PixelShader;
	InputLayout Layout;
	PrimitiveTopology Topology;
};

struct GraphicsPipelineState
{
	GraphicsPipelineStateDesc Desc;
};

}
}
}