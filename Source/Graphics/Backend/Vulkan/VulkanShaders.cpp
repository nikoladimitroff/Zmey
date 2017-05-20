#include <Zmey/Graphics/Backend/Vulkan/VulkanShaders.h>

namespace Zmey
{
namespace Graphics
{
namespace Backend
{
namespace Shaders
{
const uint32_t g_RectsVS[] = {
#include "../../Shaders/Compiled/RectsVS.h"
};
const uint32_t g_RectsVSSize = sizeof(g_RectsVS);

const uint32_t g_RectsPS[] = {
#include "../../Shaders/Compiled/RectsPS.h"
};
const uint32_t g_RectsPSSize = sizeof(g_RectsPS);
}
}
}
}