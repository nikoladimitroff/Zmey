#include <Zmey/Graphics/Backend/Vulkan/VulkanShaders.h>

#ifdef USE_VULKAN

namespace Zmey
{
namespace Graphics
{
namespace Backend
{
namespace Shaders
{
const uint32_t g_UIVS[] = {
#include "../../Shaders/Compiled/Spir-V/UIVS.h"
};
const uint32_t g_UIVSSize = sizeof(g_UIVS);

const uint32_t g_UIPS[] = {
#include "../../Shaders/Compiled/Spir-V/UIPS.h"
};
const uint32_t g_UIPSSize = sizeof(g_UIPS);

const uint32_t g_MeshVS[] = {
#include "../../Shaders/Compiled/Spir-V/MeshVS.h"
};
const uint32_t g_MeshVSSize = sizeof(g_MeshVS);

const uint32_t g_MeshPS[] = {
#include "../../Shaders/Compiled/Spir-V/MeshPS.h"
};
const uint32_t g_MeshPSSize = sizeof(g_MeshPS);
}
}
}
}

#endif