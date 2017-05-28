#pragma once
#include <stdint.h>
#include <bitset>

#include <Zmey/Hash.h>

namespace Zmey
{
enum class KeyboardButton : uint8_t
{
	Shift = 16, Alt, Ctrl,
	Digit0 = 48, Digit1, Digit2, Digit3, Digit4, Digit5, Digit6, Digit7, Digit8, Digit9,
	A = 65, B, C, D, E, F, G, H, I, J, K, L, M,
	N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
	Numpad0 = 96, Numpad1, Numpad2, Numpad3, Numpad4, Numpad5, Numpad6, Numpad7, Numpad8, Numpad9,
	F1 = 112, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12
};

enum class GamepadButton : uint8_t
{
	DpadUp, DpadDown, DpadLeft, DpadRight,
	FaceUp, FaceDown, FaceLeft, FaceRight,
	LeftShoulder, RightShoulder, LeftTrigger, RightTrigger
};

enum class GamepadAxis : uint8_t
{
	LeftStickX,
	LeftStickY,
	RightStickX,
	RightStickY,
	LeftTrigger,
	RightTrigger
};

enum class MouseButton : uint8_t
{
	LeftButton,
	RightButton,
	MiddleButton
};
enum class MouseAxis : uint8_t
{
	X,
	Y,
	Wheel
};

struct InputState
{
	inline bool IsButtonPressed(MouseButton button)
	{
		return MouseButtons[static_cast<uint8_t>(button)];
	}
	bool KeyboardButtons(KeyboardButton button)
	{
		return MouseButtons[static_cast<uint8_t>(button)];
	}
	bool IsButtonPressed(GamepadButton button)
	{
		return GamepadButtons[static_cast<uint8_t>(button)];
	}
	float ReadAxis(MouseAxis axis)
	{
		return MouseAxes[static_cast<uint8_t>(axis)];
	}
	float ReadAxis(GamepadAxis axis)
	{
		return GamepadAxes[static_cast<uint8_t>(axis)];
	}
private:
	float GamepadAxes[8];
	float MouseAxes[4];
	std::bitset<128> KeyboardButtons;
	std::bitset<32> GamepadButtons;
	std::bitset<4> MouseButtons;
};

using InputActionHandler = void(*)(const InputState&);

class InputController
{
public:
	void DispatchActionEventsForFrame();
	void AddListenerForAction(Zmey::Hash actionName, InputActionHandler actionHandler);
	void RemoveListenerForAction(Zmey::Hash actionName, InputActionHandler actionHandler);

};

}