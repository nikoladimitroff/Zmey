#pragma once
#include <string>
#include <type_traits>

#include <Zmey/Memory/MemoryManagement.h>
#include <Zmey/Hash.h>

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

	void Reserve(uint64_t requestedSize)
	{
		GrowIfNeeded(requestedSize);
	}
	void Write(const uint8_t* data, uint64_t size)
	{
		GrowIfNeeded(m_Size + size);
		std::memcpy(m_Buffer.get() + m_Size, data, size);
		m_Size += size;
	}
	const uint8_t* GetData() const
	{
		return m_Buffer.get();
	}
	const uint64_t GetDataSize() const
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
		uint64_t stringText = std::strlen(value);
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
	void GrowIfNeeded(uint64_t requestedSize)
	{
		uint64_t newCapacity = m_Capacity;
		while (newCapacity < requestedSize)
		{
			newCapacity *= 2;
		}
		if (newCapacity != m_Capacity)
		{
			stl::unique_array<uint8_t> newBuffer = stl::make_unique_array<uint8_t>(newCapacity);
			std::memcpy(newBuffer.get(), m_Buffer.get(), m_Size);
			std::swap(m_Buffer, newBuffer);
			m_Capacity = newCapacity;
		}
	}

	uint64_t m_Size;
	uint64_t m_Capacity;
	stl::unique_array<uint8_t> m_Buffer;
};

class MemoryInputStream
{
public:
	MemoryInputStream(const uint8_t* buffer, uint64_t bufferSize)
		: m_ReaderPosition(0)
		, m_BufferSize(bufferSize)
		, m_Buffer(buffer)
	{}

	void Read(uint8_t* bufferToFill, uint64_t bytesToRead)
	{
		ASSERT_FATAL(m_ReaderPosition + bytesToRead <= m_BufferSize);
		std::memcpy(bufferToFill, m_Buffer + m_ReaderPosition, bytesToRead);
		m_ReaderPosition += bytesToRead;
	}

	bool IsEOF() const
	{
		return m_ReaderPosition >= m_BufferSize;
	}

	template<typename T>
	friend typename std::enable_if<std::is_integral<T>::value, MemoryInputStream&>::type
		operator>>(MemoryInputStream& stream, T& value)
	{
		value = *reinterpret_cast<const T*>(stream.m_Buffer + stream.m_ReaderPosition);
		stream.m_ReaderPosition += sizeof(T);
		return stream;
	}
	friend MemoryInputStream& operator>>(MemoryInputStream& stream, Zmey::Name& value)
	{
		std::memcpy(&value, stream.m_Buffer + stream.m_ReaderPosition, sizeof(Zmey::Name));
		stream.m_ReaderPosition += sizeof(Zmey::Name);
		return stream;
	}
	template<typename Allocator>
	friend MemoryInputStream& operator>>(MemoryInputStream& stream, std::basic_string<char, std::char_traits<char>, Allocator>& value)
	{
		const char* stringPtr = reinterpret_cast<const char*>(stream.m_Buffer + stream.m_ReaderPosition);
		uint64_t stringLength = std::strlen(stringPtr);
		value.assign(stringPtr, stringLength);
		stream.m_ReaderPosition += stringLength + 1;
		return stream;
	}
private:
	uint64_t m_ReaderPosition;
	uint64_t m_BufferSize;
	const uint8_t* m_Buffer;
};

}
