#include <Zmey/Config.h>
#include "WindowsPlatform.h" // TODO(alex): should we include like this ? 


#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

namespace Zmey
{
WindowHandle WindowsPlatform::SpawnWindow(unsigned width, unsigned height, const char* title)
{
	return WindowHandle(0);
}

void WindowsPlatform::DestroyWindow(WindowHandle handle)
{

}
};