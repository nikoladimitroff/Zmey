#pragma once
#include <stdint.h>
#include <functional>

#include <Zmey/Memory/MemoryManagement.h>

#pragma warning(push)
#pragma warning(disable: 4307) // Integral constant overflow
namespace Zmey
{
struct Hash
{
	constexpr Hash(uint64_t value) : m_Value(value) {}

	template<typename T>
	constexpr Hash(T input) : m_Value(HashFor<T>(input)) {}

	constexpr bool operator==(const Hash& other) const
	{
		return m_Value == other.m_Value;
	}
	constexpr bool operator!=(const Hash& other) const
	{
		return !(m_Value == other.m_Value);
	}
	constexpr bool operator<(const Hash& other) const
	{
		return m_Value < other.m_Value;
	}
	constexpr explicit operator uint64_t() const
	{
		return m_Value;
	}
private:
	uint64_t m_Value;
	friend struct std::hash<Zmey::Hash>;
};


template<typename T>
constexpr uint64_t HashFor(T input)
{
	// If compilation fails here, you are missing a hash function for your type
	static_assert(true);
}

template<>
constexpr uint64_t HashFor<const char*>(const char* input)
{
	uint64_t hash = 0xcbf29ce484222325;
	const uint64_t prime = 0x00000100000001b3;

	while (*input) {
		hash ^= static_cast<uint64_t>(*input);
		hash *= prime;
		++input;
	}

	return hash;
}

namespace HashHelpers
{
struct CaseInsensitiveStringWrapper
{
	constexpr CaseInsensitiveStringWrapper(const char* input)
		: Input(input)
	{}
	const char* Input;
};
}

inline constexpr char ToLower(const char c)
{
	return (c >= 'A' && c <= 'Z') ? c + ('a' - 'A') : c;
}
template<>
inline constexpr uint64_t HashFor<HashHelpers::CaseInsensitiveStringWrapper>(HashHelpers::CaseInsensitiveStringWrapper caseInsenstiveString)
{
	uint64_t hash = 0xcbf29ce484222325;
	const uint64_t prime = 0x00000100000001b3;

	const char* input = caseInsenstiveString.Input;
	while (*input) {
		hash ^= static_cast<uint64_t>(ToLower(*input));
		hash *= prime;
		++input;
	}

	return hash;
}

class Name
{
public:
	constexpr Name()
		: m_Hash(0ull)
	{}

	constexpr Name(const char* str)
		: m_Hash(HashHelpers::CaseInsensitiveStringWrapper(str))
	{}
	constexpr Name(const stl::string& str)
		: m_Hash(HashHelpers::CaseInsensitiveStringWrapper(str.c_str()))
	{}
	explicit constexpr Name(Zmey::Hash hash)
		: m_Hash(hash)
	{}

	constexpr bool operator==(const Name& other) const
	{
		return m_Hash == other.m_Hash;
	}
	constexpr bool operator!=(const Name& other) const
	{
		return !(m_Hash == other.m_Hash);
	}
	constexpr bool operator<(const Name& other) const
	{
		return m_Hash < other.m_Hash;
	}
	constexpr explicit operator uint64_t() const
	{
		return static_cast<uint64_t>(m_Hash);
	}

	static constexpr Name NullName()
	{
		Name nullName(0ull);
		return nullName;
	}
private:
	constexpr Name(uint64_t value)
		: m_Hash(value)
	{}
	Zmey::Hash m_Hash;
	friend struct std::hash<Zmey::Name>;
};

}

namespace std
{
	template<>
	struct hash<Zmey::Hash>
	{
		inline constexpr size_t operator()(const Zmey::Hash& x) const
		{
			return x.m_Value;
		}
	};
	template<>
	struct hash<Zmey::Name>
	{
		inline constexpr size_t operator()(const Zmey::Name& x) const
		{
			return static_cast<uint64_t>(x.m_Hash);
		}
	};
}

#pragma warning(pop)