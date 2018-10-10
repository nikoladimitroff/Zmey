#pragma once

namespace Zmey
{
using WindowHandle = size_t;

//TODO(alex): ugly hack remove me
extern bool g_Run;

class IPlatform
{
public:
	virtual ~IPlatform() {}

	// The name is SpawnWindow because CreateWindow is macro in Windows.h
	// Creates a new window with given params
	virtual WindowHandle SpawnWindow(unsigned width, unsigned height, const char* title) = 0;

	// Pumps message for this window handle
	virtual void PumpMessages(WindowHandle handle) = 0;

	// Destroys the window
	virtual void KillWindow(WindowHandle handle) = 0;
};
};