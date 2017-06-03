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

}
}