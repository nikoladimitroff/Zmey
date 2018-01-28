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
};

struct Material
{
	Color BaseColorFactor;
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