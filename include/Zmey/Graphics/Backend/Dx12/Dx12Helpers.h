#pragma once

#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl.h>
using Microsoft::WRL::ComPtr;

#include <Zmey/Logging.h>

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

}
}
}