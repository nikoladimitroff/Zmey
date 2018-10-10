#pragma once

#include <stdint.h>
#include <Zmey/Memory/MemoryManagement.h>
#include <Zmey/Graphics/Backend/BackendDeclarations.h>

namespace Zmey
{
namespace Graphics
{
class UploadHeap
{
public:
	UploadHeap();
	void CopyDataToTexture(Backend::CommandList* list, unsigned size, const void* data, Backend::Texture* texture);
private:
	stl::vector<Backend::Buffer*> m_Chunks;
};

}
}