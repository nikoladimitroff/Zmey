#pragma once

#include <Zmey/Memory/MemoryManagement.h>
#include <Zmey/Graphics/GraphicsObjects.h>
#include <Zmey/Math/Math.h>
#include <Zmey/Graphics/View.h>

namespace Zmey
{
namespace Graphics
{

// TODO: this needs better memory management.
// Use some kind of block allocations
struct FrameData
{
	uint64_t FrameIndex;

	// TODO(alex): handle multiple views
	ViewType Type;
	Vector3 EyePosition;
	Matrix4x4 ProjectionMatrix;
	Matrix4x4 ViewMatrix;
	unsigned Width;
	unsigned Height;

	// Data for render
	stl::vector<MeshHandle> MeshHandles;
	stl::vector<Matrix4x4> MeshTransforms;

	// UI Renderer data
	stl::vector<uint8_t> UIVertexData;
	stl::vector<uint8_t> UIIndexData;
	stl::vector<uint8_t> UIDrawData;
	stl::vector<uint32_t> UIDrawVertexOffset; // TODO: this has some duplicated values and is somewhat wastefull
};

}
}