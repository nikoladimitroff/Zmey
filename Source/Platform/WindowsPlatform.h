#pragma once

#include <Zmey/Config.h>
#include <Zmey/Platform/Platform.h>

namespace Zmey
{
class WindowsPlatform : public IPlatform
{
public:
	virtual WindowHandle SpawnWindow(unsigned width, unsigned height, const char* title) override;
	virtual void PumpMessages(WindowHandle handle) override;
	virtual void KillWindow(WindowHandle handle) override;
	virtual void SetWindowTitle(WindowHandle handle, const char* title) override;
};
};