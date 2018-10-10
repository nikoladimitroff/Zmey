#pragma once

#include <Zmey/Config.h>
#ifdef USE_VULKAN

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <functional>

#include <Zmey/Graphics/GraphicsObjects.h>
#include <Zmey/Logging.h>

namespace Zmey
{
namespace Graphics
{
namespace Backend
{

template <typename T, typename Deleter>
class VulkanUniqueHandle
{
public:
	VulkanUniqueHandle(const Deleter& deleter)
		: m_Handle(VK_NULL_HANDLE)
		, m_Deleter(deleter)
	{}

	VulkanUniqueHandle(const T& value = VK_NULL_HANDLE, const Deleter& deleter = Deleter())
		: m_Handle(value)
		, m_Deleter(deleter)
	{}

	~VulkanUniqueHandle()
	{
		DestroyHandle();
	}

	VulkanUniqueHandle(VulkanUniqueHandle<T, Deleter>&& other)
		: m_Handle(other.m_Handle)
		, m_Deleter(other.m_Deleter)
	{
		other.m_Handle = VK_NULL_HANDLE;
	}

	VulkanUniqueHandle& operator=(VulkanUniqueHandle<T, Deleter>&& other)
	{
		std::swap(m_Handle, other.m_Handle);
		std::swap(m_Deleter, other.m_Deleter);
	}

	VulkanUniqueHandle& operator=(const VulkanUniqueHandle&) = delete;
	VulkanUniqueHandle(const VulkanUniqueHandle&) = delete;

	const T* operator&() const
	{
		return &m_Handle;
	}

	T* Receive()
	{
		DestroyHandle();
		return &m_Handle;
	}

	operator T() const
	{
		return m_Handle;
	}

private:
	T m_Handle;
	Deleter m_Deleter;

	void DestroyHandle()
	{
		if (m_Handle != VK_NULL_HANDLE)
		{
			m_Deleter(m_Handle);
			m_Handle = VK_NULL_HANDLE;
		}
	}
};

#ifdef _DEBUG
extern PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT;
#endif

#define DECLARE_DELETER_AND_UNIQUE_HANDLE(name) \
struct Vulkan##name##Deleter; \
using Vulkan##name##Handle = VulkanUniqueHandle<Vk##name, Vulkan##name##Deleter>

#define DEFINE_DELETER(name) \
struct Vulkan##name##Deleter \
{ \
	void operator()(Vk##name value) \
	{ \
		vkDestroy##name(value, nullptr); \
	} \
}

#define DEFINE_DELETER_WITH_EXTRA(name, extra) \
struct Vulkan##name##Deleter \
{ \
	const Vk##extra* extra; \
	Vulkan##name##Deleter(const Vk##extra* e) : extra(e) {} \
	void operator()(Vk##name value) \
	{ \
		vkDestroy##name(*extra, value, nullptr); \
	} \
}


DECLARE_DELETER_AND_UNIQUE_HANDLE(Instance);
DECLARE_DELETER_AND_UNIQUE_HANDLE(Device);
DECLARE_DELETER_AND_UNIQUE_HANDLE(Pipeline);
DECLARE_DELETER_AND_UNIQUE_HANDLE(DebugReportCallbackEXT);
DECLARE_DELETER_AND_UNIQUE_HANDLE(SurfaceKHR);
DECLARE_DELETER_AND_UNIQUE_HANDLE(SwapchainKHR);
DECLARE_DELETER_AND_UNIQUE_HANDLE(ImageView);
DECLARE_DELETER_AND_UNIQUE_HANDLE(CommandPool);
DECLARE_DELETER_AND_UNIQUE_HANDLE(ShaderModule);

DEFINE_DELETER(Instance);
DEFINE_DELETER(Device);

DEFINE_DELETER_WITH_EXTRA(Pipeline, Device);
DEFINE_DELETER_WITH_EXTRA(DebugReportCallbackEXT, Instance);
DEFINE_DELETER_WITH_EXTRA(SurfaceKHR, Instance);

DEFINE_DELETER_WITH_EXTRA(SwapchainKHR, Device);
DEFINE_DELETER_WITH_EXTRA(ImageView, Device);
DEFINE_DELETER_WITH_EXTRA(CommandPool, Device);
DEFINE_DELETER_WITH_EXTRA(ShaderModule, Device);

#undef DECLARE_DELETER_AND_UNIQUE_HANDLE
#undef DEFINE_DELETER
#undef DEFINE_DELETER_WITH_EXTRA

inline VkFormat PixelFormatToVulkan(PixelFormat format)
{
	switch (format)
	{
	case PixelFormat::B8G8R8A8:
		return VK_FORMAT_B8G8R8A8_UNORM;
	default:
		NOT_REACHED();
		return VK_FORMAT_UNDEFINED;
	}
}

}
}
}
#endif