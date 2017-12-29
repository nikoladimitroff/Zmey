#pragma once

#include <Zmey/Graphics/Managers/TextureManager.h>
#include <Zmey/Graphics/Backend/Device.h>

namespace Zmey
{
namespace Graphics
{

uint64_t TextureManager::s_TextureNextId = 0u;

TextureManager::TextureManager(Backend::Device* device)
	: m_Device(device)
{}

void TextureManager::DestroyResources()
{
	for (auto& buff : m_Textures)
	{
		//m_Device->DestroyTexture(buff.second);
	}
}

TextureHandle TextureManager::CreateTexture(uint32_t size, void* data)
{
	TextureHandle handle = s_TextureNextId++;
	//auto Texture = m_Device->CreateTexture(size);

	//auto memory = Texture->Map();
	//memcpy(memory, data, size);
	//Texture->Unmap();

	//m_Textures[handle] = Texture;
	return handle;
}

const Backend::Texture* TextureManager::GetTexture(TextureHandle handle) const
{
	auto findIt = m_Textures.find(handle);
	ASSERT_RETURN_VALUE(findIt != m_Textures.end(), nullptr);
	return findIt->second;
}

}
}