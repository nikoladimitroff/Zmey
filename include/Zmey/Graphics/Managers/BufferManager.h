#pragma once

#include <Zmey/Graphics/GraphicsObjects.h>
#include <Zmey/Graphics/Backend/BackendDeclarations.h>
#include <Zmey/Memory/MemoryManagement.h>

namespace Zmey
{
namespace Graphics
{

class BufferManager
{
public:
	BufferManager(Backend::Device* device);

	void DestroyResources();

	BufferHandle CreateStaticBuffer(Backend::BufferUsage usage, uint32_t size, void* data);

	const Backend::Buffer* GetBuffer(BufferHandle handle) const;
private:
	stl::unordered_map<BufferHandle, Backend::Buffer*> m_Buffers;
	Backend::Device* m_Device;
	static uint64_t s_BufferNextId;
};

}
}