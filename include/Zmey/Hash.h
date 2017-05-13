#pragma once
#include <stdint.h>

namespace Zmey
{

using HashType = uint64_t;
constexpr HashType Hash(const char* input)
{
	HashType hash = 0xcbf29ce484222325;
	const HashType prime = 0x00000100000001b3;

	while (*input) {
		hash ^= static_cast<HashType>(*input);
		hash *= prime;
		++input;
	}

	return hash;
}

}