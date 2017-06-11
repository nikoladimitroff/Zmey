#include <Zmey/Config.h>
#include <Zmey/Modules.h>

#include "WindowsPlatform.h" // TODO(alex): should we include like this ? 


#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <windowsx.h>
#include <Xinput.h>

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

void UpdateInputControllerFromGamepad(const XINPUT_STATE& gamepadState, uint8_t playerIndex)
{
	auto& input = *Zmey::Modules::InputController;
	
	uint16_t xinputGamepadButtons[] =
	{
		XINPUT_GAMEPAD_DPAD_UP, XINPUT_GAMEPAD_DPAD_DOWN, XINPUT_GAMEPAD_DPAD_LEFT, XINPUT_GAMEPAD_DPAD_RIGHT,
		XINPUT_GAMEPAD_START, XINPUT_GAMEPAD_BACK,
		XINPUT_GAMEPAD_LEFT_THUMB, XINPUT_GAMEPAD_RIGHT_THUMB,
		XINPUT_GAMEPAD_LEFT_SHOULDER, XINPUT_GAMEPAD_RIGHT_SHOULDER,
		XINPUT_GAMEPAD_A, XINPUT_GAMEPAD_B, XINPUT_GAMEPAD_X, XINPUT_GAMEPAD_Y,
	};
	GamepadButton zmeyGamepadButtons[] =
	{
		GamepadButton::DpadUp, GamepadButton::DpadDown, GamepadButton::DpadLeft, GamepadButton::DpadRight,
		GamepadButton::Start, GamepadButton::Back,
		GamepadButton::LeftStick, GamepadButton::RightStick,
		GamepadButton::LeftShoulder, GamepadButton::RightShoulder,
		GamepadButton::FaceDown, GamepadButton::FaceRight, GamepadButton::FaceLeft, GamepadButton::FaceUp
	};
	static_assert(sizeof(xinputGamepadButtons) / sizeof(uint16_t) == sizeof(zmeyGamepadButtons) / sizeof(GamepadButton), "The arrays above must have the same size");
	for (size_t i = 0; i < sizeof(xinputGamepadButtons) / sizeof(uint16_t); i++)
	{
		input.SetButtonPressed(zmeyGamepadButtons[i], gamepadState.Gamepad.wButtons & xinputGamepadButtons[i], playerIndex);
	}
	const float NormalizeBy32K = 1 / 32767.0f;
	const float NormalizeBy255 = 1 / 255.0f;
	input.SetAxis(GamepadAxis::LeftStickX, gamepadState.Gamepad.sThumbLX * NormalizeBy32K, playerIndex);
	input.SetAxis(GamepadAxis::LeftStickY, gamepadState.Gamepad.sThumbLY * NormalizeBy32K, playerIndex);
	input.SetAxis(GamepadAxis::RightStickX, gamepadState.Gamepad.sThumbRX * NormalizeBy32K, playerIndex);
	input.SetAxis(GamepadAxis::RightStickY, gamepadState.Gamepad.sThumbRY * NormalizeBy32K, playerIndex);
	input.SetAxis(GamepadAxis::LeftTriggerAxis, gamepadState.Gamepad.bLeftTrigger * NormalizeBy255, playerIndex);
	input.SetAxis(GamepadAxis::RightTriggerAxis, gamepadState.Gamepad.bRightTrigger * NormalizeBy255, playerIndex);
	input.SetButtonPressed(GamepadButton::LeftTrigger, gamepadState.Gamepad.bLeftTrigger >= XINPUT_GAMEPAD_TRIGGER_THRESHOLD, playerIndex);
	input.SetButtonPressed(GamepadButton::RightTrigger, gamepadState.Gamepad.bRightTrigger >= XINPUT_GAMEPAD_TRIGGER_THRESHOLD, playerIndex);
}

void WindowsPlatform::PumpMessages(WindowHandle handle)
{
	MSG msg;

	while(PeekMessage(&msg, HWND(handle), 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	for (uint8_t i = 0; i < XUSER_MAX_COUNT; ++i)
	{
		XINPUT_STATE gamepadState;
		ZeroMemory(&gamepadState, sizeof(XINPUT_STATE));
		DWORD result = XInputGetState(i, &gamepadState);
		if (result == ERROR_SUCCESS)
		{
			// TODO: Detect a disconnected controller and nullify all of its inputs
			UpdateInputControllerFromGamepad(gamepadState, i);
		}
	}
}

void WindowsPlatform::KillWindow(WindowHandle handle)
{
	DestroyWindow(HWND(handle));
}
};