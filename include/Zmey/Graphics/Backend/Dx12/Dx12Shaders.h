#pragma once

#include <Zmey/Config.h>
#ifdef USE_DX12

#include <stdint.h>

#include <Zmey/Graphics/Backend/Dx12/Dx12Helpers.h>

namespace Zmey
{
namespace Graphics
{
namespace Backend
{
namespace Shaders
{
namespace UI
{
#include "../../Source/Graphics/Shaders/Compiled/DirectX/UIVS.h"
#include "../../Source/Graphics/Shaders/Compiled/DirectX/UIPS.h"
}

namespace Mesh
{
#include "../../Source/Graphics/Shaders/Compiled/DirectX/MeshVS.h"
#include "../../Source/Graphics/Shaders/Compiled/DirectX/MeshPS.h"
}
}
}
}
}

#endif