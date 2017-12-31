#pragma once

#include <Zmey/Memory/MemoryManagement.h>
#include <Zmey/Graphics/GraphicsObjects.h>
#include <Zmey/Math/Math.h>
#include <Zmey/Graphics/View.h>

namespace Zmey
{
namespace Graphics
{

// TODO(alex): delete this
struct Rect
{
	float x, y, width, height; // In normalized space
	float color[4]; // R, G, B, A
};

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
	TextureHandle TextureToUse; // TODO: remove me

	// Data for render
	stl::vector<MeshHandle> MeshHandles;
	stl::vector<Matrix4x4> MeshTransforms;
	stl::vector<Vector3> MeshColors;

	stl::vector<Rect> RectsToDraw;
};

}
}