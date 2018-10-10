#include <Zmey/Graphics/Managers/UploadHeap.h>
#include <Zmey/Graphics/RendererGlobals.h>
#include <Zmey/Graphics/Backend/Device.h>
#include <Zmey/Graphics/Backend/Buffer.h>
#include <Zmey/Graphics/Backend/Texture.h>
#include <Zmey/Graphics/Backend/CommandList.h>

namespace Zmey
{
namespace Graphics
{
UploadHeap::UploadHeap()
{}

void UploadHeap::CopyDataToTexture(Backend::CommandList* list, unsigned size, const void* data, Backend::Texture* texture)
{
	// Create new chunk with data
	auto buffer = Globals::g_Device->CreateBuffer(size, Backend::BufferUsage::Copy);

	auto memory = buffer->Map();
	memcpy(memory, data, size);
	buffer->Unmap();

	list->CopyBufferToTexture(buffer, texture);

	m_Chunks.push_back(buffer);
}
}
}