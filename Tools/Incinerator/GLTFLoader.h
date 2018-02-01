#pragma once

#include <stdint.h>
#include <string>
#include <vector>
#include <unordered_set>

namespace Zmey
{
namespace Incinerator
{
namespace GLTFLoader
{
bool ParseAndIncinerate(const uint8_t* gltfData,
	uint32_t gltfSize,
	const std::string& destinationFolder,
	const std::string& contentFolder,
	const std::vector<std::string>& meshFiles,
	std::unordered_set<std::string>& additionalResources);
};
}
}