#pragma once
#include <cstddef>
#include <cassert>

namespace Zmey
{
// StdAllocatorTemplate uses the curiously-recurring template pattern idiom.
// AllocatorImpl must implement Initialize, Malloc, Free and Realloc
template<class AllocatorImpl, class T>
struct StlAllocatorTemplate
{
	typedef T value_type;
	typedef value_type* pointer;
	typedef const value_type* const_pointer;
	typedef value_type& reference;
	typedef const value_type& const_reference;
	typedef std::size_t size_type;
	typedef std::ptrdiff_t difference_type;

	template<typename U>
	struct rebind
	{
		typedef StlAllocatorTemplate<AllocatorImpl, U> other;
	};

	StlAllocatorTemplate()
	{
		m_Impl.Initialize();
	}
	template <class U>
	StlAllocatorTemplate(const StlAllocatorTemplate<AllocatorImpl, U>& /*other*/)
	{
	}
	T* allocate(std::size_t n)
	{
		return reinterpret_cast<T*>(m_Impl.Malloc(n * sizeof(T), alignof(T)));
	}
	void deallocate(T* p, std::size_t)
	{
		m_Impl.Free(p);
	}
private:
	AllocatorImpl m_Impl;
};
template <class AllocatorImpl, class T, class U>
bool operator==(const StlAllocatorTemplate<AllocatorImpl, T>& lhs, const StlAllocatorTemplate<AllocatorImpl, U>& rhs)
{
	return &lhs == &rhs;
}
template <class AllocatorImpl, class T, class U>
bool operator!=(const StlAllocatorTemplate<AllocatorImpl, T>& lhs, const StlAllocatorTemplate<AllocatorImpl, U>& rhs)
{
	return !(lhs == rhs);
}

}