#pragma once
#include <array>
#include <deque>
#include <memory>
#include <string>
#include <vector>
#include <queue>
#include <unordered_map>

#include <Zmey/Memory/Allocator.h>
#include "StlAllocator.h"
#include "LinearAllocator.h"
#include "PoolAllocator.h"

namespace Zmey
{
extern Zmey::IAllocator* GAllocator;

inline void* ZmeyMalloc(size_t size)
{
	return GAllocator->Malloc(size, 0);
}

inline void* ZmeyRealloc(void* ptr, size_t size)
{
	return GAllocator->Realloc(ptr, size);
}

inline void ZmeyFree(void* ptr)
{
	GAllocator->Free(ptr);
}

extern Zmey::StaticDataAllocator<1024 * 8> GStaticDataAllocator;
template<typename T>
inline T* StaticAlloc()
{
	return new (GStaticDataAllocator.Malloc(sizeof(T), alignof(T))) T();
}

constexpr size_t tls_TempAllocatorSize = 4 * 1024 * 1024; // 4MB
using TempAllocator = ThreadLocalLinearAllocator<tls_TempAllocatorSize>;

using ArraySizeType = unsigned;
template<typename T>
inline T* ZmeyMallocArray(unsigned count)
{
	const auto totalSize = sizeof(T) * count + sizeof(ArraySizeType);
	auto ptr = ZmeyMalloc(totalSize);
	*static_cast<ArraySizeType*>(ptr) = static_cast<ArraySizeType>(count);

	auto result = reinterpret_cast<T*>(static_cast<char*>(ptr) + sizeof(ArraySizeType));
	if (!std::is_fundamental<T>::value)
	{
		for (auto i = 0u; i < count; ++i)
		{
			new(result + i) T();
		}
	}
	return result;
}


template<typename T>
inline void ZmeyDestroyArray(const T* ptr)
{
	if (!ptr)
		return nullptr;

	const ArraySizeType* counterPtr = reinterpret_cast<ArraySizeType*>(ptr);
	const auto count = *counterPtr;

	for (int i = int(count - 1); i >= 0; --i) {
		(ptr + i)->~T();
	}
}
	
namespace tmp
{

	template<typename Base>
	struct TempDeleter
	{
		TempDeleter() {}
		template<typename Derived>
		TempDeleter(const TempDeleter<Derived>&)
		{
			static_assert(std::is_base_of<Base, Derived>::value, "Only inherited casting is allowed");
			static_assert(std::has_virtual_destructor<Base>::value, "Type needs virtual destructor!");
		}
		void operator()(Base* ptr)
		{
			ptr->~Base();
			TempAllocator::GetTlsAllocator().Free(ptr);
		}
	};
	template<typename T>
	struct TempDeleterArray
	{
		TempDeleterArray() {}
		template<typename Derived>
		TempDeleterArray(const TempDeleterArray<Derived>&)
		{
			static_assert(std::is_base_of<Base, Derived>::value, "Only inherited casting is allowed");
			static_assert(std::has_virtual_destructor<Base>::value, "Type needs virtual destructor!");
		}
		void operator()(T* ptr)
		{
			ZmeyDestroyArray(ptr);
		}
	};

	template<typename T>
	using vector = std::vector<T, StlAllocatorTemplate<TempAllocator, T>>;
	template<typename T>
	using small_vector = vector<T>;
	using string = std::basic_string<char, std::char_traits<char>, StlAllocatorTemplate<TempAllocator, char>>;
	using wstring = std::basic_string<wchar_t, std::char_traits<wchar_t>, StlAllocatorTemplate<TempAllocator, wchar_t>>;
	using small_string = string;
	template<typename T>
	using unique_ptr = std::unique_ptr<T, TempDeleter<T>>;
	template<typename T>
	using unique_array = std::unique_ptr<T[], TempDeleterArray<T>>;
	template<typename T>
	using shared_ptr = std::shared_ptr<T>;
	template<typename T, typename... Args>
	inline unique_ptr<T> make_unique(Args&&... args)
	{
		auto ptr = new(TempAllocator::GetTlsAllocator().Malloc(sizeof(T)) T(std::forward<Args>(args)...));
		return unique_ptr<T>(ptr);
	}
	template<typename T, typename... Args>
	inline unique_array<T> make_unique_array(Args&&... args)
	{
		auto ptr = new(TempAllocator::GetTlsAllocator().Malloc(sizeof(T)) T(std::forward<Args>(args)...));
		return unique_array<T>(ptr);
	}
	// Convert to intrusive ptr
	template<typename T, typename... Args>
	inline shared_ptr<T> make_shared(Args&&... args)
	{
		auto ptr = new(TempAllocator::GetTlsAllocator().Malloc(sizeof(T)) T(std::forward<Args>(args)...));
		return shared_ptr<T>(ptr);
	}
}

class DefaultAllocator
{
public:
	void Initialize()
	{}
	inline void* Malloc(size_t size, unsigned alignment)
	{
		auto ptr = GAllocator->Malloc(size, alignment);
		return ptr;
	}
	inline void Free(void* ptr)
	{
		GAllocator->Free(ptr);
	}
	inline void* Realloc(void* ptr, size_t newSize)
	{
		return GAllocator->Realloc(ptr, newSize);
	}
};

namespace stl
{
	template<typename Base>
	struct StdDeleter
	{
		StdDeleter() {}
		template<typename Derived>
		StdDeleter(const StdDeleter<Derived>&)
		{
			static_assert(std::is_base_of<Base, Derived>::value, "Only inherited casting is allowed");
			static_assert(std::has_virtual_destructor<Base>::value, "Type needs virtual destructor!");
		}
		void operator()(Base* ptr)
		{
			ptr->~Base();
			ZmeyFree(ptr);
		}
	};
	template<typename Base>
	struct StdDeleterArray
	{
		StdDeleterArray() {}
		template<typename Derived>
		StdDeleterArray(const StdDeleterArray<Derived>&)
		{
			static_assert(std::is_base_of<Base, Derived>::value, "Only inherited casting is allowed");
			static_assert(std::has_virtual_destructor<Base>::value, "Type needs virtual destructor!");
		}
		void operator()(Base* ptr)
		{
			ZmeyDestroyArray(ptr);
		}
	};

	template<typename T, size_t Size>
	using array = std::array<T, Size>;
	template<typename T>
	using vector = std::vector<T, StlAllocatorTemplate<DefaultAllocator, T>>;
	template<typename T>
	using small_vector = vector<T>;
	template<typename T>
	using deque = std::deque<T, StlAllocatorTemplate<DefaultAllocator, T>>;
	template<typename T>
	using queue = std::queue<T, stl::deque<T>>;
	template<typename K, typename V>
	using unordered_map = std::unordered_map<K, V, std::hash<K>, std::equal_to<K>, StlAllocatorTemplate<DefaultAllocator, std::pair<const K, V>>>;
	using string = std::basic_string<char, std::char_traits<char>, StlAllocatorTemplate<DefaultAllocator, char>>;
	using wstring = std::basic_string<wchar_t, std::char_traits<wchar_t>, StlAllocatorTemplate<DefaultAllocator, wchar_t>>;
	using small_string = string;
	template<typename T>
	using unique_ptr = std::unique_ptr<T, StdDeleter<T>>;
	template<class T>
	using unique_array = std::unique_ptr<T[], StdDeleterArray<T>>;
	template<typename T>
	using shared_ptr = std::shared_ptr<T>;
	template<typename T, typename... Args>
	inline unique_ptr<T> make_unique(Args&&... args)
	{
		auto ptr = new (GAllocator->Malloc(sizeof(T), alignof(T))) T(std::forward<Args>(args)...);
		return unique_ptr<T>(ptr);
	}
	template<typename T, typename... Args>
	inline unique_array<T> make_unique_array(Args&&... args)
	{
		auto ptr = new (GAllocator->Malloc(sizeof(T), alignof(T))) T(std::forward<Args>(args)...);
		return unique_array<T>(ptr);
	}
	template<typename T, typename... Args>
	inline shared_ptr<T> make_shared(Args&&... args)
	{
		auto ptr = new (GAllocator->Malloc(sizeof(T), alignof(T))) T(std::forward<Args>(args)...);
		return shared_ptr<T>(ptr);
	}
}

namespace pool
{
	template<typename T, unsigned short ObjectCount = 256>
	using vector = std::vector<T, StlAllocatorTemplate<PoolAllocator<T, ObjectCount>, T>>;
}
}
