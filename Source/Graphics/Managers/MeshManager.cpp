#pragma once

#include <Zmey/Graphics/Managers/MeshManager.h>

namespace Zmey
{
namespace Graphics
{

uint64_t MeshManager::s_MeshNextId = 0;

MeshHandle MeshManager::CreateMesh(Mesh mesh)
{
	auto id = s_MeshNextId++;
	m_Meshes[id] = mesh;
	return id;
}

const Mesh* MeshManager::GetMesh(MeshHandle handle) const
{
	auto findIt = m_Meshes.find(handle);
	ASSERT_RETURN_VALUE(findIt != m_Meshes.end(), nullptr);
	return &findIt->second;
}

}
}