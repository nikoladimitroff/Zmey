#pragma once

#include "TextureLoader.h"
#include <memory>
#include <mutex>

#include <DirectXTex/DirectXTex.h>

namespace Zmey
{
namespace Incinerator
{
namespace TextureLoader
{
void ConvertImageToDDSMemory(const std::string& filename, std::vector<uint8_t>& dds)
{
	static std::once_flag flag;
	std::call_once(flag, []() {
		// Initialize COM (needed for WIC)
		HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
		assert(SUCCEEDED(hr));
	});

	auto sizeNeeded = ::MultiByteToWideChar(CP_UTF8, 0, filename.c_str(), filename.size(), nullptr, 0);

	std::wstring wFilename(sizeNeeded, 0);
	::MultiByteToWideChar(CP_UTF8, 0, filename.c_str(), filename.size(), &wFilename[0], sizeNeeded);

	DirectX::TexMetadata info;
	DirectX::ScratchImage image;
	auto hr = DirectX::LoadFromWICFile(
		wFilename.c_str(),
		DirectX::TEX_FILTER_DEFAULT,
		&info,
		image);
	assert(SUCCEEDED(hr));

	DirectX::Blob ddsBlob;
	hr = DirectX::SaveToDDSMemory(
		*image.GetImage(0, 0, 0),
		DirectX::DDS_FLAGS_NONE,
		ddsBlob);
	assert(SUCCEEDED(hr));

	auto oldSize = dds.size();
	dds.resize(oldSize + ddsBlob.GetBufferSize());
	std::memcpy(dds.data() + oldSize, ddsBlob.GetBufferPointer(), ddsBlob.GetBufferSize());
}
};
}
}