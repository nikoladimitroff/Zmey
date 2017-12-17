#pragma once

#include <Zmey/Graphics/Managers/BufferManager.h>
#include <Zmey/Graphics/Backend/Backend.h>

namespace Zmey
{
namespace Graphics
{

uint64_t BufferManager::s_BufferNextId = 0u;

BufferManager::BufferManager(Backend::Backend* backend)
	: m_Backend(backend)
{}

void BufferManager::DestroyResources()
{
	for (auto& buff : m_Buffers)
	{
		m_Backend->DestroyBuffer(buff.second);
	}
}

BufferHandle BufferManager::CreateStaticBuffer(Backend::BufferUsage usage, uint32_t size, void* data)
{
	BufferHandle handle = s_BufferNextId++;
	auto buffer = m_Backend->CreateBuffer(size, usage);

	auto memory = buffer->Map();
	memcpy(memory, data, size);
	buffer->Unmap();

	m_Buffers[handle] = buffer;
	return handle;
}

const Backend::Buffer* BufferManager::GetBuffer(BufferHandle handle) const
{
	auto findIt = m_Buffers.find(handle);
	ASSERT_RETURN_VALUE(findIt != m_Buffers.end(), nullptr);
	return findIt->second;
}

}
}