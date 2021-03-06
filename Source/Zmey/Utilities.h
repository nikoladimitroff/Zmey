#pragma once

#include <Zmey/Memory/MemoryManagement.h>

namespace Zmey
{
namespace Utilities
{

template<typename Allocator>
tmp::small_vector<tmp::string> SplitString(const std::basic_string<char, std::char_traits<char>, Allocator>& str, const char delimiter)
{
	tmp::small_vector<tmp::string> result;
	size_t start = 0U;
	size_t end = str.find(delimiter);
	if (end == std::string::npos)
	{
		result.push_back(tmp::string(str.data(), str.size()));
		return result;
	}
	while (end != std::string::npos)
	{
		result.push_back(tmp::string(&str[start], end - start));
		start = end + 1;
		end = str.find(delimiter, start);
	}
	result.push_back(tmp::string(&str[start], str.size() - start));
	return result;
}

template<typename CharType, typename Allocator>
inline bool EndsWith(const std::basic_string<CharType, std::char_traits<CharType>, Allocator>& value, const std::basic_string<CharType, std::char_traits<CharType>, Allocator>& ending)
{
	if (ending.size() > value.size()) return false;
	return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}


template<typename CharType, typename Allocator>
inline bool EndsWith(const std::basic_string<CharType, std::char_traits<CharType>, Allocator>& value, const CharType* endingPtr)
{
	std::basic_string<CharType, std::char_traits<CharType>, Allocator> ending(endingPtr);
	return EndsWith(value, ending);
}

template<typename T>
class ConstructorInitializable
{
public:
	ConstructorInitializable()
		: m_IsInitialized(false)
	{}
	T& operator=(const T& other)
	{
		assert(!m_IsInitialized);
		std::memcpy(m_Buffer, &other, sizeof(T));
		m_IsInitialized = true;
		return *this;
	}
	inline operator T&()
	{
		return reinterpret_cast<T&>(m_Buffer);
	}
	inline operator const T&() const
	{
		return reinterpret_cast<T&>(m_Buffer);
	}
private:
	char m_Buffer[sizeof(T)];
	bool m_IsInitialized : 1;
};

}
}