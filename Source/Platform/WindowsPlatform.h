#pragma once

#include <Zmey/Config.h>
#include <Zmey/Platform/Platform.h>

namespace Zmey
{
class WindowsPlatform : public IPlatform
{
public:
	virtual WindowHandle SpawnWindow(unsigned width, unsigned height, const char* title) override;
	virtual void DestroyWindow(WindowHandle handle) override;
};
};