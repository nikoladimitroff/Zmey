#pragma once

#include <stdint.h>
#include <Zmey/Logging.h>
#include <Zmey/Math/Math.h>
#include <Zmey/Memory/MemoryManagement.h>
#include <Zmey/Graphics/ResourceSet.h>
#include <Zmey/Graphics/Backend/CommandList.h>

#include <numeric>

namespace Zmey
{
namespace Graphics
{
namespace Backend
{

inline uint32_t GetPushConstantCountForResourceSet(ResourceSetType type)
{
	switch (type)
	{
	case ResourceSetType::Light:
		return 2 * 4; // LightDir + EyePosition (Vector4)
	case ResourceSetType::Transform:
		return 2 * 16; // WVP + WV Matrices (Matrix4x4)
	case ResourceSetType::Material:
		return 4; // Color (Vector4)
	case ResourceSetType::UIPosition:
		return 4; // Scale + Translate (Vector2)
	case ResourceSetType::UITexture:
		return 0; // This is only texture
	default:
		NOT_REACHED();
		return 0;
	}
}

inline uint32_t GatherPushConstantCount(const stl::vector<ResourceSetType>& sets)
{
	return std::accumulate(sets.begin(), sets.end(), uint32_t(0), [](uint32_t result, ResourceSetType set) {
		return result + GetPushConstantCountForResourceSet(set);
	});
}
}
}
}