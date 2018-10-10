#include <Zmey/ResourceLoader/DDSLoader.h>

namespace Zmey
{

namespace
{
enum DDSFlags
{
	DDS_Caps       = 0x00000001,
	DDS_Height     = 0x00000002,
	DDS_Width      = 0x00000004,
	DDS_PixelFomat = 0x00001000,
};

enum DDSPixelFormat
{
	DDS_AlphaPixels = 0x00000001,
	DDS_RGB         = 0x00000040,
	DDS_RGBA        = DDS_RGB | DDS_AlphaPixels,
};
}

DDSLoader::DDSLoader(const uint8_t* data, size_t size)
{
	auto header = reinterpret_cast<const DDS_HEADER*>(data + 4);

	const uint32_t alwaysRequiredFlags = DDS_Caps | DDS_Height | DDS_Width | DDS_PixelFomat;

	if (size >= sizeof(DDS_HEADER) + 4 // Check if the data is more than the header
		&& data[0] == 'D' && data[1] == 'D' && data[2] == 'S' && data[3] == ' ' // Check magic number
		&& header->Size == sizeof(DDS_HEADER) // Header size is right
		&& header->DDSpf.Size == sizeof(DDS_PIXELFORMAT) // Pixel format size is right
		&& (header->Flags & alwaysRequiredFlags) == alwaysRequiredFlags)
	{
		// TODO(alex): check for DX10 header
		m_DDSHeader = header;
	}
}

Graphics::PixelFormat DDSLoader::GetPixelFormat()
{
	auto format = Graphics::PixelFormat::Unknown;

	if ((m_DDSHeader->DDSpf.Flags & DDS_RGBA) == DDS_RGBA
		&& m_DDSHeader->DDSpf.RGBBitCount == 32
		&& m_DDSHeader->DDSpf.RBitMask == 0x00FF0000
		&& m_DDSHeader->DDSpf.GBitMask == 0x0000FF00
		&& m_DDSHeader->DDSpf.BBitMask == 0x000000FF
		&& m_DDSHeader->DDSpf.ABitMask == 0xFF000000)
	{
		format = Graphics::PixelFormat::B8G8R8A8;
	}

	return format;
}

uint32_t DDSLoader::GetWidth()
{
	return m_DDSHeader->Width;
}

uint32_t DDSLoader::GetHeight()
{
	return m_DDSHeader->Height;
}

const uint8_t* DDSLoader::GetImageData()
{
	return reinterpret_cast<const uint8_t*>(m_DDSHeader) + sizeof(DDS_HEADER);
}
}