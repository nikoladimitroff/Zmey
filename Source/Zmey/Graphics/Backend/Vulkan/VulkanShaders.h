#pragma once

#include <Zmey/Config.h>
#ifdef USE_VULKAN

#include <stdint.h>

namespace Zmey
{
namespace Graphics
{
namespace Backend
{
namespace Shaders
{
extern const uint32_t g_MeshVS[];
extern const uint32_t g_MeshVSSize;
extern const uint32_t g_MeshPS[];
extern const uint32_t g_MeshPSSize;
extern const uint32_t g_UIVS[];
extern const uint32_t g_UIVSSize;
extern const uint32_t g_UIPS[];
extern const uint32_t g_UIPSSize;
}
}
}
}

#endif