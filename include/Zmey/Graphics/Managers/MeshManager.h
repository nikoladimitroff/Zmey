#pragma once

#include <Zmey/Graphics/GraphicsObjects.h>
#include <Zmey/Memory/MemoryManagement.h>
#include <Zmey/Math/Math.h>

namespace Zmey
{
namespace Graphics
{

struct MeshVertex
{
	Vector3 Position;
	Vector3 Normal;
	Vector2 TextureUV;
};

struct MeshDataHeader
{
	uint64_t VerticesCount;
	uint64_t IndicesCount;
	// MeshVertex[VerticesCount]
	// uint32_t[IndicesCount]
};

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