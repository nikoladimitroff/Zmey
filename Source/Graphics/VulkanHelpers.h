#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <functional>

namespace Zmey
{
namespace Graphics
{

template <typename T>
class VDeleter
{
public:
	VDeleter()
		: VDeleter([](T, VkAllocationCallbacks*) {})
	{}

	VDeleter(std::function<void(T, VkAllocationCallbacks*)> deleteFunction)
	{
		m_Deleter = [=](T obj) {
			deleteFunction(obj, nullptr);
		};
	}

	VDeleter(const VDeleter<VkInstance>& instance, std::function<void(VkInstance, T, VkAllocationCallbacks*)> deleteFunction)
	{
		m_Deleter = [&instance, deleteFunction](T obj) {
			deleteFunction(instance, obj, nullptr);
		};
	}

	VDeleter(const VDeleter<VkDevice>& device, std::function<void(VkDevice, T, VkAllocationCallbacks*)> deleteFunction) {
		m_Deleter = [&device, deleteFunction](T obj) {
			deleteFunction(device, obj, nullptr);
		};
	}

	~VDeleter()
	{
		Cleanup();
	}

	const T* operator&() const
	{
		return &m_Object;
	}

	T* Replace()
	{
		Cleanup();
		return &m_Object;
	}

	operator T() const
	{
		return m_Object;
	}

	void operator=(T rhs)
	{
		if (rhs != m_Object)
		{
			Cleanup();
			m_Object = rhs;
		}
	}

	template <typename V>
	bool operator==(V rhs)
	{
		return m_Object == T(rhs);
	}

private:
	T m_Object{ VK_NULL_HANDLE };
	std::function<void(T)> m_Deleter;

	void Cleanup()
	{
		if (m_Object != VK_NULL_HANDLE)
		{
			m_Deleter(m_Object);
			m_Object = VK_NULL_HANDLE;
		}
	}
};

}
}