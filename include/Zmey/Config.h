#pragma once


#if defined(_WIN32)
	#define ZMEY_PLATFORM_WIN 1
#elif defined(__linux__)
	#define ZMEY_PLATFORM_LINUX 1
#elif defined(__APPLE_CC__) || defined(__APPLE_CPP__)
	#define ZMEY_PLATFORM_MAC 1
#endif

#if !defined(ZMEY_IMPORT_SYMBOL)
	#if defined(ZMEY_PLATFORM_WIN)
		#define ZMEY_EXPORT_SYMBOL __declspec(dllexport)
		#define ZMEY_IMPORT_SYMBOL __declspec(dllimport)
	#elif defined(ZMEY_PLATFORM_POSIX)
		#define ZMEY_EXPORT_SYMBOL __attribute__ ((visibility ("default")))
		#define ZMEY_IMPORT_SYMBOL
	#elif defined(ZMEY_PLATFORM_IOS)
		#define ZMEY_EXPORT_SYMBOL
		#define ZMEY_IMPORT_SYMBOL
	#endif
#endif

#ifdef ZMEY_EXPORTS
	#define ZMEY_API ZMEY_EXPORT_SYMBOL
#else
	#define ZMEY_API ZMEY_IMPORT_SYMBOL
#endif

// Renderer config
// Uncomment for dx12
//#define USE_DX12

// Assume if we haven't choosen what to use that only vulkan is present
// as Dx12 runs only on Win10
#if !defined(USE_DX12) && !defined(USE_VULKAN)
#define USE_VULKAN
#endif

#if defined(USE_DX12) && defined(USE_VULKAN)
#error Cannot Use Dx12 and Vulkan at the same time
#endif