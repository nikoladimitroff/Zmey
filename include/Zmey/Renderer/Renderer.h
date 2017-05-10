#pragma once

#include <Zmey/Platform/Platform.h>

namespace Zmey
{

class IRenderer
{
public:
	virtual ~IRenderer() {}

	virtual bool CreateWindowSurface(WindowHandle handle) = 0;
	virtual void ClearBackbufferSurface(float color[4]) = 0;

	virtual void DrawScene() = 0;
};

};