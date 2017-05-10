#pragma once

#include <Zmey/Platform/Platform.h>
#include <Zmey/Memory/MemoryManagement.h>
#include <cstdint>

namespace Zmey
{
namespace Graphics
{

struct RendererData;
struct FrameData;

class RendererInterface
{
public:
	RendererInterface();

	bool CreateWindowSurface(WindowHandle handle);

	void RenderFrame(FrameData& frameData);

	bool CheckIfFrameCompleted(uint64_t frameIndex);
private:
	stl::unique_ptr<RendererData> m_Data;
};

}
}
