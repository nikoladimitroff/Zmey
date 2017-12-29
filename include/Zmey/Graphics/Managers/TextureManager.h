#pragma once

#include <Zmey/Graphics/GraphicsObjects.h>
#include <Zmey/Graphics/Backend/BackendDeclarations.h>
#include <Zmey/Memory/MemoryManagement.h>

namespace Zmey
{
namespace Graphics
{

class TextureManager
{
public:
	TextureManager(Backend::Device* backend);

	void DestroyResources();

	TextureHandle CreateTexture(uint32_t width, uint32_t height, PixelFormat format, void* data);

	const Backend::Texture* GetTexture(TextureHandle handle) const;
private:
	stl::unordered_map<TextureHandle, Backend::Texture*> m_Textures;
	Backend::Device* m_Device;
	static uint64_t s_TextureNextId;
};

}
}