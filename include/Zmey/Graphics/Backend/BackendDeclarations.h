#pragma once

namespace Zmey
{
namespace Graphics
{
namespace Backend
{

struct GraphicsPipelineState;
struct Framebuffer;

class CommandList;

class Device;

class Buffer;
enum class BufferUsage
{
	Vertex,
	Index,
	Copy
};

struct Texture;


}
}
}