#pragma once

namespace Zmey
{
using WindowHandle = size_t;

class IPlatform
{
public:
	virtual ~IPlatform() {}

	// The name is SpawnWindow because CreateWindow is macro in Windows.h
	// Creates a new window with given params
	virtual WindowHandle SpawnWindow(unsigned width, unsigned height, const char* title) = 0;

	// Destroys the window
	virtual void DestroyWindow(WindowHandle handle) = 0;

};
};