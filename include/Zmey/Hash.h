#pragma once
#include <stdint.h>

namespace Zmey
{

struct Hash
{
	constexpr Hash(uint64_t value) : m_Value(value) {}

	template<typename T>
	constexpr Hash(T input) : m_Value(HashFor<T>(input)) {}

	bool operator==(const Hash& other) const
	{
		return m_Value == other.m_Value;
	}
	bool operator<(const Hash& other) const
	{
		return m_Value < other.m_Value;
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
inline constexpr uint64_t HashFor<const char*>(const char* input)
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

template<>
inline constexpr uint64_t HashFor<HashHelpers::CaseInsensitiveStringWrapper>(HashHelpers::CaseInsensitiveStringWrapper caseInsenstiveString)
{
	uint64_t hash = 0xcbf29ce484222325;
	const uint64_t prime = 0x00000100000001b3;

	const char* input = caseInsenstiveString.Input;
	while (*input) {
		hash ^= static_cast<uint64_t>(tolower(*input));
		hash *= prime;
		++input;
	}

	return hash;
}

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
}
