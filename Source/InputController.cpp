#pragma once
#include <Zmey/InputController.h>

#include <algorithm>

#include <Zmey/Logging.h>
#include <Zmey/Modules.h>

#include <Zmey/Utilities.h>
#include <Zmey/Math/Math.h>

namespace Zmey
{

ActionMapping::ActionMapping(const char* actionName, const tmp::small_vector<Binding> bindings)
	: ActionNameHash(Zmey::HashHelpers::CaseInsensitiveStringWrapper(actionName))
{
	size_t sizeToCopy = std::min(bindings.size(), static_cast<size_t>(MaxKeyBindingsPerAction)) * sizeof(Binding);
	std::memcpy(const_cast<Binding*>(&ActionBindings[0]), bindings.data(), sizeToCopy);
}
bool ActionMapping::Binding::MatchesInput(const InputState& current, const InputState& previous, float& outAxisValue) const
{
	bool matchesModifiers =
		current.IsButtonPressed(KeyboardButton::Ctrl) == ExpectsCtrl &&
		current.IsButtonPressed(KeyboardButton::Shift) == ExpectsShift &&
		current.IsButtonPressed(KeyboardButton::Alt) == ExpectsAlt;

	switch (Type)
	{
	case MappingType::MouseButton:
		outAxisValue = 1.f;
		return matchesModifiers &&
			current.IsButtonPressed(static_cast<Zmey::MouseButton>(InputEnumData)) &&
			(IsContinuous || !previous.IsButtonPressed(static_cast<Zmey::MouseButton>(InputEnumData)));
	case MappingType::KeyboardButton:
		outAxisValue = 1.f;
		return matchesModifiers &&
			current.IsButtonPressed(static_cast<Zmey::KeyboardButton>(InputEnumData)) &&
			(IsContinuous || !previous.IsButtonPressed(static_cast<Zmey::KeyboardButton>(InputEnumData)));
	case MappingType::GamepadButton:
		outAxisValue = 1.f;
		return matchesModifiers &&
			current.IsButtonPressed(static_cast<Zmey::GamepadButton>(InputEnumData)) &&
			(IsContinuous || !previous.IsButtonPressed(static_cast<Zmey::GamepadButton>(InputEnumData)));
	case MappingType::MouseAxis:
	{
		outAxisValue = current.ReadAxis(static_cast<Zmey::MouseAxis>(InputEnumData));
		float oldAxisValue = previous.ReadAxis(static_cast<Zmey::MouseAxis>(InputEnumData));
		return matchesModifiers &&
			(IsContinuous || !FloatClose(outAxisValue, oldAxisValue));
	}
	case MappingType::GamepadAxis:
	{
		outAxisValue = current.ReadAxis(static_cast<Zmey::GamepadAxis>(InputEnumData));
		float oldAxisValue = previous.ReadAxis(static_cast<Zmey::GamepadAxis>(InputEnumData));
		return matchesModifiers &&
			(IsContinuous || !FloatClose(outAxisValue, oldAxisValue));
	}
	default:
		NOT_REACHED();
		return false;
	}
}

void ConvertKeynameToBinding(Zmey::Hash keyHash, ActionMapping::Binding& binding)
{
	ActionMapping::Binding preConvertState = binding;

	using IHash = Zmey::HashHelpers::CaseInsensitiveStringWrapper;
#define CHECK_KEYNAME(Category, Keyname) \
	case static_cast<uint64_t>(Hash(IHash(#Keyname))): \
		binding.InputEnumData = static_cast<uint8_t>(Category::##Keyname); \
		binding.Type = ActionMapping::MappingType::##Category; \
	break

#pragma warning(push)
#pragma warning(disable: 4307) // Integral constant overflow
	switch (static_cast<uint64_t>(keyHash))
	{
		CHECK_KEYNAME(MouseAxis, MouseX);
		CHECK_KEYNAME(MouseAxis, MouseY);
		CHECK_KEYNAME(MouseAxis, Wheel);

		CHECK_KEYNAME(GamepadAxis, LeftStickX);
		CHECK_KEYNAME(GamepadAxis, LeftStickY);
		CHECK_KEYNAME(GamepadAxis, RightStickX);
		CHECK_KEYNAME(GamepadAxis, RightStickY);
		CHECK_KEYNAME(GamepadAxis, LeftTriggerAxis);
		CHECK_KEYNAME(GamepadAxis, RightTriggerAxis);

		CHECK_KEYNAME(MouseButton, LeftButton);
		CHECK_KEYNAME(MouseButton, MiddleButton);
		CHECK_KEYNAME(MouseButton, RightButton);

		CHECK_KEYNAME(GamepadButton, DpadUp);
		CHECK_KEYNAME(GamepadButton, DpadDown);
		CHECK_KEYNAME(GamepadButton, DpadLeft);
		CHECK_KEYNAME(GamepadButton, DpadRight);
		CHECK_KEYNAME(GamepadButton, Start);
		CHECK_KEYNAME(GamepadButton, Back);
		CHECK_KEYNAME(GamepadButton, FaceUp);
		CHECK_KEYNAME(GamepadButton, FaceDown);
		CHECK_KEYNAME(GamepadButton, FaceLeft);
		CHECK_KEYNAME(GamepadButton, FaceRight);
		CHECK_KEYNAME(GamepadButton, LeftStick);
		CHECK_KEYNAME(GamepadButton, RightStick);
		CHECK_KEYNAME(GamepadButton, LeftShoulder);
		CHECK_KEYNAME(GamepadButton, RightShoulder);
		CHECK_KEYNAME(GamepadButton, LeftTrigger);
		CHECK_KEYNAME(GamepadButton, RightTrigger);

		CHECK_KEYNAME(KeyboardButton, Digit0);
		CHECK_KEYNAME(KeyboardButton, Digit1);
		CHECK_KEYNAME(KeyboardButton, Digit2);
		CHECK_KEYNAME(KeyboardButton, Digit3);
		CHECK_KEYNAME(KeyboardButton, Digit4);
		CHECK_KEYNAME(KeyboardButton, Digit5);
		CHECK_KEYNAME(KeyboardButton, Digit6);
		CHECK_KEYNAME(KeyboardButton, Digit7);
		CHECK_KEYNAME(KeyboardButton, Digit8);
		CHECK_KEYNAME(KeyboardButton, Digit9);
		CHECK_KEYNAME(KeyboardButton, A);
		CHECK_KEYNAME(KeyboardButton, B);
		CHECK_KEYNAME(KeyboardButton, C);
		CHECK_KEYNAME(KeyboardButton, D);
		CHECK_KEYNAME(KeyboardButton, E);
		CHECK_KEYNAME(KeyboardButton, F);
		CHECK_KEYNAME(KeyboardButton, G);
		CHECK_KEYNAME(KeyboardButton, H);
		CHECK_KEYNAME(KeyboardButton, I);
		CHECK_KEYNAME(KeyboardButton, J);
		CHECK_KEYNAME(KeyboardButton, K);
		CHECK_KEYNAME(KeyboardButton, L);
		CHECK_KEYNAME(KeyboardButton, M);
		CHECK_KEYNAME(KeyboardButton, N);
		CHECK_KEYNAME(KeyboardButton, O);
		CHECK_KEYNAME(KeyboardButton, P);
		CHECK_KEYNAME(KeyboardButton, Q);
		CHECK_KEYNAME(KeyboardButton, R);
		CHECK_KEYNAME(KeyboardButton, S);
		CHECK_KEYNAME(KeyboardButton, T);
		CHECK_KEYNAME(KeyboardButton, U);
		CHECK_KEYNAME(KeyboardButton, V);
		CHECK_KEYNAME(KeyboardButton, W);
		CHECK_KEYNAME(KeyboardButton, X);
		CHECK_KEYNAME(KeyboardButton, Y);
		CHECK_KEYNAME(KeyboardButton, Z);

		CHECK_KEYNAME(KeyboardButton, Numpad0);
		CHECK_KEYNAME(KeyboardButton, Numpad1);
		CHECK_KEYNAME(KeyboardButton, Numpad2);
		CHECK_KEYNAME(KeyboardButton, Numpad3);
		CHECK_KEYNAME(KeyboardButton, Numpad4);
		CHECK_KEYNAME(KeyboardButton, Numpad5);
		CHECK_KEYNAME(KeyboardButton, Numpad6);
		CHECK_KEYNAME(KeyboardButton, Numpad7);
		CHECK_KEYNAME(KeyboardButton, Numpad8);
		CHECK_KEYNAME(KeyboardButton, Numpad9);

		CHECK_KEYNAME(KeyboardButton, F1);
		CHECK_KEYNAME(KeyboardButton, F2);
		CHECK_KEYNAME(KeyboardButton, F3);
		CHECK_KEYNAME(KeyboardButton, F4);
		CHECK_KEYNAME(KeyboardButton, F5);
		CHECK_KEYNAME(KeyboardButton, F6);
		CHECK_KEYNAME(KeyboardButton, F7);
		CHECK_KEYNAME(KeyboardButton, F8);
		CHECK_KEYNAME(KeyboardButton, F9);
		CHECK_KEYNAME(KeyboardButton, F10);
		CHECK_KEYNAME(KeyboardButton, F11);
		CHECK_KEYNAME(KeyboardButton, F12);
	}
#undef CHECK_KEYNAME

#define CHECK_MODIFIER(Keyname) \
	case static_cast<uint64_t>(Hash(IHash(#Keyname))): \
		binding.Expects##Keyname = true; \
	break

	switch (static_cast<uint64_t>(keyHash))
	{
		CHECK_MODIFIER(Ctrl);
		CHECK_MODIFIER(Alt);
		CHECK_MODIFIER(Shift);
	}
	if (keyHash == Zmey::Hash(IHash("CONT")))
	{
		binding.IsContinuous = true;
	}
#pragma warning(pop)

	// Sanity check that we actually did something
	ASSERT_FATAL(preConvertState != binding);
}

InputController::InputController()
{
	auto settings = Zmey::Modules::SettingsManager->DataFor("InputController");
	auto actions = settings->ReadValue("Actions");
	for (const tmp::string& actionCommands : actions)
	{
		auto commandParts = Zmey::Utilities::SplitString(actionCommands, ',');
		const tmp::string& actionName = commandParts[0];
		tmp::small_vector<ActionMapping::Binding> bindings;
		for (size_t i = 1 /* the action name is 0*/; i < commandParts.size(); ++i)
		{
			// Split each binding by +
			auto bindingParts = Zmey::Utilities::SplitString(commandParts[i], '+');
			ActionMapping::Binding binding;
			for (tmp::string& part : bindingParts)
			{
				Zmey::Hash partHash(Zmey::HashHelpers::CaseInsensitiveStringWrapper(part.c_str()));
				ConvertKeynameToBinding(partHash, binding);
			}
			bindings.push_back(binding);
		}
		ActionMapping mapping(actionName.c_str(), bindings);
		m_ActionMappings.push_back(std::move(mapping));
	}
}

void InputController::DispatchActionEventsForFrame()
{
	for (const auto& mapping : m_ActionMappings)
	{
		float axisValue;
		bool actionFired = std::any_of(std::begin(mapping.ActionBindings),
			std::end(mapping.ActionBindings),
			[this, &axisValue](const ActionMapping::Binding& binding)
			{ return binding.MatchesInput(m_CurrentState, m_PreviousState, axisValue); });
		if (actionFired)
		{
			auto& handlers = m_ActionHandlers[mapping.ActionNameHash];
			for (auto handler : handlers)
			{
				handler(axisValue);
			}
		}
	}
	m_PreviousState = m_CurrentState;
}
void InputController::AddListenerForAction(Zmey::Hash actionName, InputActionDelegate actionHandler)
{
	m_ActionHandlers[actionName].push_back(actionHandler);
}
void InputController::RemoveListenerForAction(Zmey::Hash actionName, InputActionDelegate actionHandler)
{
	ASSERT_FATAL("Not implemented");
}

}