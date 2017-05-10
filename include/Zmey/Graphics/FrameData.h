#pragma once

#include <Zmey/Memory/MemoryManagement.h>
#include <Zmey/Graphics/ResourceHandles.h>
#include <Zmey/Math/Math.h>

namespace Zmey
{
namespace Graphics
{

// TODO: this needs better memory management.
// Use some kind of block allocations
struct FrameData
{
	uint64_t FrameIndex;

	stl::vector<MeshHandle> MeshHandles;
	stl::vector<Vector3> MeshPositions;
};

}
}