#pragma once

#include <Zmey/Graphics/GraphicsObjects.h>
#include <Zmey/Memory/MemoryManagement.h>

namespace Zmey
{
namespace Graphics
{

struct Mesh
{
	uint32_t IndexCount;
	BufferHandle VertexBuffer;
	BufferHandle IndexBuffer;
};

class MeshManager
{
public:
	MeshHandle CreateMesh(Mesh mesh);
	const Mesh* GetMesh(MeshHandle handle) const;
private:
	stl::unordered_map<MeshHandle, Mesh> m_Meshes;
	static uint64_t s_MeshNextId;
};

}
}