#pragma once
#include <stdint.h>
#include <bitset>
#include <functional>

#include <Zmey/Hash.h>
#include <Zmey/Memory/MemoryManagement.h>

namespace Zmey
{
enum class KeyboardButton : uint8_t
{
	Shift = 16, Alt, Ctrl,
	Space = 32,
	Digit0 = 48, Digit1, Digit2, Digit3, Digit4, Digit5, Digit6, Digit7, Digit8, Digit9,
	A = 65, B, C, D, E, F, G, H, I, J, K, L, M,
	N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
	Numpad0 = 96, Numpad1, Numpad2, Numpad3, Numpad4, Numpad5, Numpad6, Numpad7, Numpad8, Numpad9,
	F1 = 112, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12
};

enum class GamepadButton : uint8_t
{
	DpadUp, DpadDown, DpadLeft, DpadRight,
	Start, Back,
	FaceUp, FaceDown, FaceLeft, FaceRight,
	LeftStick, RightStick,
	LeftShoulder, RightShoulder, LeftTrigger, RightTrigger
};

enum class GamepadAxis : uint8_t
{
	LeftStickX,
	LeftStickY,
	RightStickX,
	RightStickY,
	LeftTriggerAxis,
	RightTriggerAxis
};

enum class MouseButton : uint8_t
{
	LeftButton,
	RightButton,
	MiddleButton
};
enum class MouseAxis : uint8_t
{
	MouseX,
	MouseY,
	Wheel
};

struct InputState
{
	inline bool IsButtonPressed(MouseButton button) const
	{
		return MouseButtons[static_cast<uint8_t>(button)];
	}
	inline bool IsButtonPressed(KeyboardButton button) const
	{
		return KeyboardButtons[static_cast<uint8_t>(button)];
	}
	inline bool IsButtonPressed(GamepadButton button) const
	{
		return GamepadButtons[static_cast<uint8_t>(button)];
	}
	inline float ReadAxis(MouseAxis axis) const
	{
		return MouseAxes[static_cast<uint8_t>(axis)];
	}
	inline float ReadAxis(GamepadAxis axis) const
	{
		return GamepadAxes[static_cast<uint8_t>(axis)];
	}
	InputState& operator=(const InputState& other)
	{
		std::memcpy(GamepadAxes, other.GamepadAxes, sizeof(GamepadAxes));
		std::memcpy(MouseAxes, other.MouseAxes, sizeof(MouseAxes));
		KeyboardButtons = other.KeyboardButtons;
		GamepadButtons = other.GamepadButtons;
		MouseButtons = other.MouseButtons;
		return *this;
	}
private:
	float GamepadAxes[8];
	float MouseAxes[4];
	std::bitset<128> KeyboardButtons;
	std::bitset<32> GamepadButtons;
	std::bitset<4> MouseButtons;
	friend class InputController;
};

using InputActionDelegate = std::function<void(float axisValue)>;

struct ActionMapping
{
	enum class MappingType : uint8_t
	{
		Invalid, MouseButton, KeyboardButton, GamepadButton, MouseAxis, GamepadAxis
	};
	struct Binding
	{
		Binding()
			: Type(MappingType::Invalid)
			, InputEnumData(0u)
			, IsContinuous(false)
			, ExpectsCtrl(false)
			, ExpectsShift(false)
			, ExpectsAlt(false)
		{}
		bool operator==(const Binding& rhs)
		{
			return Type == rhs.Type &&
				InputEnumData == rhs.InputEnumData &&
				IsContinuous == rhs.IsContinuous &&
				ExpectsCtrl == rhs.ExpectsCtrl &&
				ExpectsShift == rhs.ExpectsShift &&
				ExpectsAlt == rhs.ExpectsAlt;
		}
		bool operator!=(const Binding& rhs)
		{
			return !(*this == rhs);
		}
		MappingType Type;
		uint8_t InputEnumData;
		bool IsContinuous : 1;
		bool ExpectsCtrl : 1;
		bool ExpectsShift : 1;
		bool ExpectsAlt : 1;

		bool MatchesInput(const InputState& current, const InputState& previous, float& outAxisValue) const;
	};
	ActionMapping(const char* actionName, const tmp::small_vector<Binding> bindings);
	const Zmey::Name ActionName;
	static constexpr uint8_t MaxKeyBindingsPerAction = 2u;
	const Binding ActionBindings[MaxKeyBindingsPerAction];
};

class InputController
{
public:
	InputController();
	void DispatchActionEventsForFrame();
	ZMEY_API void AddListenerForAction(Zmey::Hash actionName, InputActionDelegate actionHandler);
	ZMEY_API void RemoveListenerForAction(Zmey::Hash actionName, InputActionDelegate actionHandler);

	inline void SetButtonPressed(MouseButton button, bool isPressed)
	{
		m_CurrentState.MouseButtons[static_cast<uint8_t>(button)] = isPressed;
	}
	inline void SetButtonPressed(KeyboardButton button, bool isPressed)
	{
		m_CurrentState.KeyboardButtons[static_cast<uint8_t>(button)] = isPressed;
	}
	inline void SetButtonPressed(GamepadButton button, bool isPressed)
	{
		m_CurrentState.GamepadButtons[static_cast<uint8_t>(button)] = isPressed;
	}
	inline void SetAxis(MouseAxis axis, float value)
	{
		m_CurrentState.MouseAxes[static_cast<uint8_t>(axis)] = value;
	}
	inline void SetAxis(GamepadAxis axis, float value)
	{
		m_CurrentState.GamepadAxes[static_cast<uint8_t>(axis)] = value;
	}
private:
	InputState m_CurrentState;
	InputState m_PreviousState;
	stl::vector<ActionMapping> m_ActionMappings;
	// TODO: This collection doesn't look very performant
	stl::unordered_map<Zmey::Hash, stl::small_vector<InputActionDelegate>> m_ActionHandlers;
};

}