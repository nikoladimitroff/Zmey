#pragma once
#include <Zmey/Logging.h>

namespace Zmey
{

template<typename T, uint16_t ObjectCount>
class PoolAllocator
{
public:
	PoolAllocator()
		: m_Head(&m_Buffer[0] + reinterpret_cast<uint64_t>(&m_Buffer[0]) % alignof(T))
	{
		static_assert(sizeof(T) >= sizeof(char*), "The pool allocator does not support elements with size less than a pointer");
		for (unsigned short i = 0u; i < ObjectCount - 1; ++i)
		{
			char* block = m_Head + i * sizeof(T);
			char* nextBlock = block + sizeof(T);
			char*& blockContents = reinterpret_cast<char*&>(*block);
			blockContents = nextBlock;
		}
	}
	void Initialize()
	{}
	inline void* Malloc(size_t size, unsigned alignment)
	{
		ASSERT_FATAL(alignment == alignof(T));
		auto ptr = m_Head;
		m_Head = reinterpret_cast<char*>(*m_Head);
		return ptr;
	}
	inline void Free(void* ptr)
	{
		char* block = static_cast<char*>(ptr);
		char*& blockContents = reinterpret_cast<char*&>(*block);
		blockContents = m_Head;
		m_Head = block;
	}
	inline void* Realloc(void* ptr, size_t newSize)
	{
		ASSERT_RETURN_VALUE(false && "Realloc shouldn't be called on pool allocators!", ptr);
	}
private:
	char m_Buffer[ObjectCount * sizeof(T) + alignof(T)];
	char* m_Head;
};

}