#include <Zmey/Config.h>
#include <Zmey/Modules.h>

#include "WindowsPlatform.h" // TODO(alex): should we include like this ? 


#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <windowsx.h>

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
	case WM_KEYDOWN:
		Zmey::Modules::InputController->SetButtonPressed(static_cast<Zmey::KeyboardButton>(wParam), true);
	break;
	case WM_KEYUP:
		Zmey::Modules::InputController->SetButtonPressed(static_cast<Zmey::KeyboardButton>(wParam), false);
	break;
	case WM_MOUSEMOVE:
		Zmey::Modules::InputController->SetAxis(Zmey::MouseAxis::MouseX, static_cast<float>(GET_X_LPARAM(lParam)));
		Zmey::Modules::InputController->SetAxis(Zmey::MouseAxis::MouseY, static_cast<float>(GET_Y_LPARAM(lParam)));
	break;
	case WM_LBUTTONDOWN:
		Zmey::Modules::InputController->SetButtonPressed(Zmey::MouseButton::LeftButton, true);
	break;
	case WM_LBUTTONUP:
		Zmey::Modules::InputController->SetButtonPressed(Zmey::MouseButton::LeftButton, false);
	break;
	case WM_RBUTTONDOWN:
		Zmey::Modules::InputController->SetButtonPressed(Zmey::MouseButton::RightButton, true);
	break;
	case WM_RBUTTONUP:
		Zmey::Modules::InputController->SetButtonPressed(Zmey::MouseButton::RightButton, false);
	break;
	case WM_MBUTTONDOWN:
		Zmey::Modules::InputController->SetButtonPressed(Zmey::MouseButton::MiddleButton, true);
	break;
	case WM_MBUTTONUP:
		Zmey::Modules::InputController->SetButtonPressed(Zmey::MouseButton::MiddleButton, false);
	break;
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