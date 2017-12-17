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
const uint32_t g_RectsVS[] = {
#include "../../Shaders/Compiled/Spir-V/RectsVS.h"
};
const uint32_t g_RectsVSSize = sizeof(g_RectsVS);

const uint32_t g_RectsPS[] = {
#include "../../Shaders/Compiled/Spir-V/RectsPS.h"
};
const uint32_t g_RectsPSSize = sizeof(g_RectsPS);

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