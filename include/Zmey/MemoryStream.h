#pragma once
#include <string>
#include <type_traits>

#include <Zmey/Memory/MemoryManagement.h>

namespace Zmey
{
class MemoryStream
{
public:
	MemoryStream()
		: m_Size(0)
		, m_Capacity(256)
		, m_Buffer(stl::make_unique_array<uint8_t>(m_Capacity))
	{}

	void Reserve(size_t requestedSize)
	{
		GrowIfNeeded(requestedSize);
	}
	void Write(const uint8_t* data, size_t size)
	{
		GrowIfNeeded(m_Size + size);
		std::memcpy(m_Buffer.get(), data, size);
		m_Size += size;
	}
	const uint8_t* GetData() const
	{
		return m_Buffer.get();
	}

	template<typename T>
	friend typename std::enable_if<std::is_integral<T>::value, MemoryStream&>::type
		operator<<(MemoryStream& stream, T value)
	{
		stream.Write(reinterpret_cast<uint8_t*>(&value), sizeof(T));
		return stream;
	}
	friend MemoryStream& operator<<(MemoryStream& stream, const char* value)
	{
		size_t stringText = std::strlen(value);
		stream.Write(reinterpret_cast<const uint8_t*>(value), stringText);
		stream.Write(reinterpret_cast<const uint8_t*>("\0"), 1);
		return stream;
	}
	template<typename T, typename Allocator>
	friend MemoryStream& operator<<(MemoryStream& stream, const std::basic_string<char, std::char_traits<char>, Allocator>& value)
	{
		stream.Write(reinterpret_cast<const uint8_t*>(value.data()), value.size());
		stream.Write(reinterpret_cast<const uint8_t*>("\0"), 1);
		return stream;
	}
private:
	void GrowIfNeeded(size_t requestedSize)
	{
		size_t newCapacity = m_Capacity;
		while (newCapacity < requestedSize)
		{
			newCapacity *= 2;
		}
		if (newCapacity < requestedSize)
		{
			stl::unique_array<uint8_t> newBuffer = stl::make_unique_array<uint8_t>(newCapacity);
			std::memcpy(newBuffer.get(), m_Buffer.get(), m_Size);
			std::swap(m_Buffer, newBuffer);
			m_Capacity = newCapacity;
		}
	}

	size_t m_Size;
	size_t m_Capacity;
	stl::unique_array<uint8_t> m_Buffer;
};

}
