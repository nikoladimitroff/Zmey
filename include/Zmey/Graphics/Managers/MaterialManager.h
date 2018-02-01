#pragma once

#include <Zmey/Graphics/GraphicsObjects.h>
#include <Zmey/Memory/MemoryManagement.h>
#include <Zmey/Math/Math.h>

namespace Zmey
{
namespace Graphics
{

struct MaterialDataHeader
{
	Color BaseColorFactor;
	uint64_t BaseColorTextureOffset;
	uint64_t BaseColorTextureSize;
	// DDSHeader for BaseColorTexture
};

struct Material
{
	Color BaseColorFactor;
	TextureHandle BaseColorTexture;
};

class MaterialManager
{
public:
	MaterialManager();
	MaterialHandle CreateMaterial(const MaterialDataHeader& material);
	const Material* GetMaterial(MaterialHandle handle) const;
private:
	stl::unordered_map<MaterialHandle, Material> m_Material;
	static uint16_t s_MaterialNextId;
};

}
}