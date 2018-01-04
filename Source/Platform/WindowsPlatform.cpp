#include <Zmey/Config.h>
#include <Zmey/Modules.h>

#include "WindowsPlatform.h" // TODO(alex): should we include like this ? 


#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <windowsx.h>
#include <Xinput.h>

#include <imgui/imgui.h>

LRESULT CALLBACK DefaultWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	auto& io = ImGui::GetIO();
	switch (message)
	{
	case WM_DESTROY:
	{
		Zmey::g_Run = false;
		PostQuitMessage(0);
		return 0;
	}
	case WM_KEYDOWN:
		Zmey::Modules.InputController.SetButtonPressed(static_cast<Zmey::KeyboardButton>(wParam), true);
		if (wParam < 256)
		{
			io.KeysDown[wParam] = 1;
		}

	break;
	case WM_KEYUP:
		Zmey::Modules.InputController.SetButtonPressed(static_cast<Zmey::KeyboardButton>(wParam), false);
		if (wParam < 256)
		{
			io.KeysDown[wParam] = 0;
		}

	break;
	case WM_CHAR:
		if (wParam > 0 && wParam < 0x10000)
		{
			io.AddInputCharacter((unsigned short)wParam);
		}
		break;
	case WM_MOUSEMOVE:
		Zmey::Modules.InputController.SetAxis(Zmey::MouseAxis::MouseX, static_cast<float>(GET_X_LPARAM(lParam)));
		Zmey::Modules.InputController.SetAxis(Zmey::MouseAxis::MouseY, static_cast<float>(GET_Y_LPARAM(lParam)));
		io.MousePos.x = (signed short)(lParam);
		io.MousePos.y = (signed short)(lParam >> 16);

	break;
	case WM_LBUTTONDOWN:
		Zmey::Modules.InputController.SetButtonPressed(Zmey::MouseButton::LeftButton, true);
		io.MouseDown[0] = true;

	break;
	case WM_LBUTTONUP:
		Zmey::Modules.InputController.SetButtonPressed(Zmey::MouseButton::LeftButton, false);
		io.MouseDown[0] = false;

	break;
	case WM_RBUTTONDOWN:
		Zmey::Modules.InputController.SetButtonPressed(Zmey::MouseButton::RightButton, true);
		io.MouseDown[1] = true;

	break;
	case WM_RBUTTONUP:
		Zmey::Modules.InputController.SetButtonPressed(Zmey::MouseButton::RightButton, false);
		io.MouseDown[1] = false;

	break;
	case WM_MBUTTONDOWN:
		Zmey::Modules.InputController.SetButtonPressed(Zmey::MouseButton::MiddleButton, true);
		io.MouseDown[2] = true;

	break;
	case WM_MBUTTONUP:
		Zmey::Modules.InputController.SetButtonPressed(Zmey::MouseButton::MiddleButton, false);
		io.MouseDown[2] = false;

	break;
	case WM_MOUSEWHEEL:
		io.MouseWheel += GET_WHEEL_DELTA_WPARAM(wParam) > 0 ? +1.0f : -1.0f;
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

	// UI
	{
		// Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array that we will update during the application lifetime.
		ImGuiIO& io = ImGui::GetIO();
		io.KeyMap[ImGuiKey_Tab] = VK_TAB;
		io.KeyMap[ImGuiKey_LeftArrow] = VK_LEFT;
		io.KeyMap[ImGuiKey_RightArrow] = VK_RIGHT;
		io.KeyMap[ImGuiKey_UpArrow] = VK_UP;
		io.KeyMap[ImGuiKey_DownArrow] = VK_DOWN;
		io.KeyMap[ImGuiKey_PageUp] = VK_PRIOR;
		io.KeyMap[ImGuiKey_PageDown] = VK_NEXT;
		io.KeyMap[ImGuiKey_Home] = VK_HOME;
		io.KeyMap[ImGuiKey_End] = VK_END;
		io.KeyMap[ImGuiKey_Delete] = VK_DELETE;
		io.KeyMap[ImGuiKey_Backspace] = VK_BACK;
		io.KeyMap[ImGuiKey_Enter] = VK_RETURN;
		io.KeyMap[ImGuiKey_Escape] = VK_ESCAPE;
		io.KeyMap[ImGuiKey_A] = 'A';
		io.KeyMap[ImGuiKey_C] = 'C';
		io.KeyMap[ImGuiKey_V] = 'V';
		io.KeyMap[ImGuiKey_X] = 'X';
		io.KeyMap[ImGuiKey_Y] = 'Y';
		io.KeyMap[ImGuiKey_Z] = 'Z';
	}

	return WindowHandle(hWnd);
}

void UpdateInputControllerFromGamepad(const XINPUT_STATE& gamepadState, uint8_t playerIndex)
{
	auto& input = Zmey::Modules.InputController;
	
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

void InputController::Vibrate(uint8_t playerIndex, float leftMotorSpeed, float rightMotorSpeed)
{
	ASSERT_RETURN(playerIndex < XUSER_MAX_COUNT && leftMotorSpeed >= 0.f && leftMotorSpeed <= 1.f && rightMotorSpeed >= 0.f && rightMotorSpeed <= 1.f);
	XINPUT_VIBRATION vibration;
	vibration.wLeftMotorSpeed = static_cast<WORD>(leftMotorSpeed * 0xFFFF);
	vibration.wRightMotorSpeed = static_cast<WORD>(rightMotorSpeed * 0xFFFF);
	::XInputSetState(playerIndex, &vibration);
}

void WindowsPlatform::KillWindow(WindowHandle handle)
{
	DestroyWindow(HWND(handle));
}

};