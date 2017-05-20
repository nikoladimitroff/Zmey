#pragma once
#include <string>
#include <type_traits>

#include <Zmey/Memory/MemoryManagement.h>

namespace Zmey
{
class MemoryOutputStream
{
public:
	MemoryOutputStream()
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
		std::memcpy(m_Buffer.get() + m_Size, data, size);
		m_Size += size;
	}
	const uint8_t* GetData() const
	{
		return m_Buffer.get();
	}
	const size_t GetDataSize() const
	{
		return m_Size;
	}

	template<typename T>
	friend typename std::enable_if<std::is_integral<T>::value, MemoryOutputStream&>::type
		operator<<(MemoryOutputStream& stream, T value)
	{
		stream.Write(reinterpret_cast<uint8_t*>(&value), sizeof(T));
		return stream;
	}
	friend MemoryOutputStream& operator<<(MemoryOutputStream& stream, const char* value)
	{
		size_t stringText = std::strlen(value);
		stream.Write(reinterpret_cast<const uint8_t*>(value), stringText);
		stream.Write(reinterpret_cast<const uint8_t*>("\0"), 1);
		return stream;
	}
	template<typename Allocator>
	friend MemoryOutputStream& operator<<(MemoryOutputStream& stream, const std::basic_string<char, std::char_traits<char>, Allocator>& value)
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

class MemoryInputStream
{
public:
	MemoryInputStream(const uint8_t* buffer, size_t bufferSize)
		: m_ReaderPosition(0)
		, m_BufferSize(bufferSize)
		, m_Buffer(buffer)
	{}

	void Read(uint8_t* bufferToFill, size_t bytesToRead)
	{
		ASSERT_FATAL(m_ReaderPosition + bytesToRead < m_BufferSize);
		std::memcpy(bufferToFill, m_Buffer + m_ReaderPosition, bytesToRead);
		m_ReaderPosition += bytesToRead;
	}

	bool IsEOF() const
	{
		return m_ReaderPosition < m_BufferSize;
	}

	template<typename T>
	friend typename std::enable_if<std::is_integral<T>::value, MemoryInputStream&>::type
		operator>>(MemoryInputStream& stream, T& value)
	{
		value = *reinterpret_cast<const T*>(stream.m_Buffer + stream.m_ReaderPosition);
		stream.m_ReaderPosition += sizeof(T);
		return stream;
	}
	template<typename Allocator>
	friend MemoryInputStream& operator>>(MemoryInputStream& stream, std::basic_string<char, std::char_traits<char>, Allocator>& value)
	{
		const char* stringPtr = reinterpret_cast<const char*>(stream.m_Buffer + stream.m_ReaderPosition);
		size_t stringLength = std::strlen(stringPtr);
		value.assign(stringPtr, stringLength);
		stream.m_ReaderPosition += stringLength + 1;
		return stream;
	}
private:
	size_t m_ReaderPosition;
	size_t m_BufferSize;
	const uint8_t* m_Buffer;
};

}
