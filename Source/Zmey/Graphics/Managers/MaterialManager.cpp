#pragma once

#include <Zmey/Graphics/Managers/MaterialManager.h>
#include <Zmey/Modules.h>

namespace Zmey
{
namespace Graphics
{

MaterialManager::MaterialManager()
{
	// Add default material
	m_Material[MaterialHandle(-1)] = Material{
		Color(0.75f, 0.75f, 0.75f, 1.0f),
		TextureHandle(-1)
	};
}

MaterialHandle MaterialManager::CreateMaterial(const MaterialDataHeader& materialHeader)
{
	Material material;
	material.BaseColorFactor = materialHeader.BaseColorFactor;

	if (materialHeader.BaseColorTextureOffset != 0
		&& materialHeader.BaseColorTextureSize != 0)
	{
		material.BaseColorTexture = Modules.Renderer.TextureLoaded(
			reinterpret_cast<const uint8_t*>(&materialHeader) + materialHeader.BaseColorTextureOffset,
			materialHeader.BaseColorTextureSize);
	}
	else
	{
		material.BaseColorTexture = -1;
	}

	auto id = materialHeader.MaterialIndex;
	m_Material[id] = material;
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