#pragma once

#include <stdint.h>
#include <string>
#include <vector>

namespace Zmey
{
namespace Incinerator
{
namespace TextureLoader
{
void ConvertImageToDDSMemory(const std::string& filename,
	std::vector<uint8_t>& dds);
};
}
}