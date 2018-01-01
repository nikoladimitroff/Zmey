#pragma once

#include <Zmey/Config.h>
#ifdef USE_DX12

#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl.h>
using Microsoft::WRL::ComPtr;

#include <Zmey/Logging.h>
#include <Zmey/Graphics/GraphicsObjects.h>
#include <assert.h>

namespace Zmey
{
namespace Graphics
{
namespace Backend
{

#define STRINGIFY(x) #x
#define STR(x) STRINGIFY(x)

#define CHECK_SUCCESS(dxCall) \
	do { \
		HRESULT hr = dxCall; \
		if(FAILED(hr)) \
		{ \
			FORMAT_LOG(Fatal, Dx12, "%s failed with HRESULT Error code %lx", STR(dxCall), hr); \
			__debugbreak(); \
		} \
	} while(0)

inline DXGI_FORMAT PixelFormatToDx12(PixelFormat format)
{
	switch (format)
	{
	case PixelFormat::B8G8R8A8:
		return DXGI_FORMAT_B8G8R8A8_UNORM;
	default:
		assert(false);
		break;
	}

	return DXGI_FORMAT_UNKNOWN;
}

}
}
}

#endif