#pragma once

#include <Zmey/Graphics/Managers/MaterialManager.h>

namespace Zmey
{
namespace Graphics
{

uint16_t MaterialManager::s_MaterialNextId = 0;

MaterialManager::MaterialManager()
{
	// Add default material
	m_Material[MaterialHandle(-1)] = Material{ Color(0.75f, 0.75f, 0.75f, 1.0f) };
}

MaterialHandle MaterialManager::CreateMaterial(const MaterialDataHeader& material)
{
	auto id = s_MaterialNextId++;
	m_Material[id] = Material{ material.BaseColorFactor };
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