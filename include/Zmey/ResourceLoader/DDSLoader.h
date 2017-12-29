#pragma once

#include <stdint.h>
#include <Zmey/Graphics/GraphicsObjects.h>

namespace Zmey
{
#pragma pack(push, 1)
struct DDS_PIXELFORMAT
{
	uint32_t Size;
	uint32_t Flags;
	uint32_t FourCC;
	uint32_t RGBBitCount;
	uint32_t RBitMask;
	uint32_t GBitMask;
	uint32_t BBitMask;
	uint32_t ABitMask;
};

struct DDS_HEADER
{
	uint32_t        Size;
	uint32_t        Flags;
	uint32_t        Height;
	uint32_t        Width;
	uint32_t        PitchOrLinearSize;
	uint32_t        Depth; // only if DDS_HEADER_FLAGS_VOLUME is set in flags
	uint32_t        MipMapCount;
	uint32_t        Reserved1[11];
	DDS_PIXELFORMAT DDSpf;
	uint32_t        Caps;
	uint32_t        Caps2;
	uint32_t        Caps3;
	uint32_t        Caps4;
	uint32_t        Reserved2;
};
#pragma pack(pop)

class DDSLoader
{
public:
	DDSLoader(uint8_t* data, size_t size);

	bool IsValid() { return m_DDSHeader; }

	Graphics::PixelFormat GetPixelFormat();

	uint8_t* GetImageData(); // 0 mip 0 array slice TODO(alex): make it take an argument
	uint32_t GetWidth();
	uint32_t GetHeight();
private:
	DDS_HEADER* m_DDSHeader = nullptr;
};
}