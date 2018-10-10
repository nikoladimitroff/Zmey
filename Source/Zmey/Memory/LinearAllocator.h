#pragma once
#include <Zmey/Logging.h>

namespace Zmey
{

template<size_t Bytes>
class LinearAllocator
{
public:
	LinearAllocator()
		: m_Marker(&m_Buffer[0])
	{}
	void Initialize()
	{}
	inline void* Malloc(size_t size, unsigned/*alignment*/)
	{
		ASSERT_FATAL(m_Marker + size < &m_Buffer[Bytes]);
		auto ptr = m_Marker;
		m_Marker += size;
		return ptr;
	}
	inline void Free(void*)
	{
	}
	inline void* Realloc(void* ptr, size_t newSize)
	{
		auto copy = Malloc(newSize, 0);
		std::memcpy(copy, ptr, newSize);
		Free(ptr);
		return copy;
	}

	inline void Reset(void* ptr)
	{
		m_Marker = reinterpret_cast<char*>(ptr);
	}

	// Used to reset
	struct Scope
	{
		Scope(LinearAllocator* allocator)
			: m_Allocator(allocator)
			, m_Marker(allocator->GetMarker())
		{}
		Scope(const Scope&) = delete;
		Scope& operator=(const Scope&) = delete;
		Scope(Scope&& other)
			: m_Allocator(other.m_Allocator)
			, m_Marker(other.m_Marker)
		{
			other.m_Marker = m_Allocator->GetMarker();
		}
		~Scope()
		{
			m_Allocator->Reset(m_Marker);
		}
	private:
		LinearAllocator* m_Allocator;
		void* m_Marker;
	};
	inline Scope ScopeNow()
	{
		return std::move(Scope(this));
	}
private:
	char m_Buffer[Bytes];
	char* m_Marker;

	inline void* GetMarker() const
	{
		return m_Marker;
	}
};

template<size_t Capacity>
class ThreadLocalLinearAllocator
{
public:
	inline void Initialize() {}
	ZMEY_API inline void* Malloc(size_t size, unsigned alignment)
	{
		return tls_Alloc.Malloc(size, alignment);
	}
	ZMEY_API inline void Free(void* ptr)
	{
		return tls_Alloc.Free(ptr);
	}
	ZMEY_API inline void* Realloc(void* ptr, size_t newSize)
	{
		return tls_Alloc.Realloc(ptr, newSize);
	}
	ZMEY_API static LinearAllocator<Capacity>& GetTlsAllocator()
	{
		return tls_Alloc;
	}
private:
	thread_local static LinearAllocator<Capacity> tls_Alloc;
};

template<unsigned Capacity>
using StaticDataAllocator = LinearAllocator<Capacity>;

}