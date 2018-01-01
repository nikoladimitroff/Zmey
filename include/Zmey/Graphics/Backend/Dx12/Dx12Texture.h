#pragma once

#include <Zmey/Config.h>
#ifdef USE_DX12
#include <Zmey/Graphics/Backend/Texture.h>
#include <Zmey/Graphics/Backend/Dx12/Dx12Helpers.h>

namespace Zmey
{
namespace Graphics
{
namespace Backend
{
class Dx12Texture : public Texture
{
public:
	ComPtr<ID3D12Resource> Texture;
	D3D12_RESOURCE_STATES State;
};
}
}
}
#endif