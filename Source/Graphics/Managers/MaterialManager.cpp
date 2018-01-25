#pragma once

#include <Zmey/Graphics/Managers/MaterialManager.h>

namespace Zmey
{
namespace Graphics
{

uint64_t MaterialManager::s_MaterialNextId = 0;

MaterialHandle MaterialManager::CreateMaterial(const MaterialDataHeader& material)
{
	auto id = s_MaterialNextId++;
	m_Material[id] = Material();
	return id;
}

const Material* MaterialManager::GetMaterial(MaterialHandle handle) const
{
	auto findIt = m_Material.find(handle);
	ASSERT_RETURN_VALUE(findIt != m_Material.end(), nullptr);
	return &findIt->second;
}

}
}