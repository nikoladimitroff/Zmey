#include <Zmey/Config.h>
#include "WindowsPlatform.h" // TODO(alex): should we include like this ? 


#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

LRESULT CALLBACK DefaultWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
	{
		Zmey::g_Run = false;
		PostQuitMessage(0);
		return 0;
	}
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

namespace Zmey
{

bool g_Run = true;

WindowHandle WindowsPlatform::SpawnWindow(unsigned width, unsigned height, const char* title)
{
	auto hIntance = GetModuleHandle(NULL);
	WNDCLASSEX wc;
	ZeroMemory(&wc, sizeof(WNDCLASSEX));

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = DefaultWindowProc;
	wc.hInstance = hIntance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.lpszClassName = "ZmeyWindowClass";

	RegisterClassEx(&wc);

	auto hWnd = CreateWindowEx(NULL,
		"ZmeyWindowClass",
		title,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		width,
		height,
		NULL,
		NULL,
		hIntance,
		NULL);

	auto res = ShowWindow(hWnd, SW_RESTORE);

	return WindowHandle(hWnd);
}

void WindowsPlatform::PumpMessages(WindowHandle handle)
{
	MSG msg;

	while(PeekMessage(&msg, HWND(handle), 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

void WindowsPlatform::KillWindow(WindowHandle handle)
{
	DestroyWindow(HWND(handle));
}
};