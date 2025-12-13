#pragma once

#include "events/Event.hxx"

namespace playground::events {
	enum class InputEventType {
		KeyPressed,
		KeyReleased,
		MouseMoved,
		MouseScrolled,
		MouseButtonPressed,
		MouseButtonReleased,
		ControllerButtonPressed,
		ControllerButtonReleased,
		ControllerAxisMoved,
		ControllerConnected,
		ControllerDisconnected
	};

    class InputEvent : public Event {
    public:
		InputEventType InputType;

        InputEvent(InputEventType inputType) : Event(EventType::Input), InputType(inputType) {}
    };

	class KeyPressedEvent : public InputEvent {
	public:
		int Key;

		KeyPressedEvent(int key, int repeatCount) : InputEvent(InputEventType::KeyPressed), Key(key) {}
	};

	class KeyReleasedEvent : public InputEvent {
	public:
		int Key;

		KeyReleasedEvent(int key) : InputEvent(InputEventType::KeyReleased), Key(key) {}
	};

	class MouseMovedEvent : public InputEvent {
	public:
		float X, Y;

		MouseMovedEvent(float x, float y) : InputEvent(InputEventType::MouseMoved), X(x), Y(y) {}
	};

	class MouseScrolledEvent : public InputEvent {
	public:
		float XOffset, YOffset;

		MouseScrolledEvent(float xOffset, float yOffset) : InputEvent(InputEventType::MouseScrolled), XOffset(xOffset), YOffset(yOffset) {}
	};

	class MouseButtonPressedEvent : public InputEvent {
	public:
		int Button;

		MouseButtonPressedEvent(int button) : InputEvent(InputEventType::MouseButtonPressed), Button(button) {}
	};

	class MouseButtonReleasedEvent : public InputEvent {
	public:
		int Button;

		MouseButtonReleasedEvent(int button) : InputEvent(InputEventType::MouseButtonReleased), Button(button) {}
	};

	class ControllerButtonPressedEvent : public InputEvent {
	public:
		int Button;

		ControllerButtonPressedEvent(int button) : InputEvent(InputEventType::ControllerButtonPressed), Button(button) {}
	};

	class ControllerButtonReleasedEvent : public InputEvent {
	public:
		int Button;

		ControllerButtonReleasedEvent(int button) : InputEvent(InputEventType::ControllerButtonReleased), Button(button) {}
	};

	class ControllerAxisMovedEvent : public InputEvent {
	public:
		int Axis;
		float Value;

		ControllerAxisMovedEvent(int axis, float value) : InputEvent(InputEventType::ControllerAxisMoved), Axis(axis), Value(value) {}
	};

	class ControllerConnectedEvent : public InputEvent {
	public:
		int Controller;

		ControllerConnectedEvent(int controller) : InputEvent(InputEventType::ControllerConnected), Controller(controller) {}
	};

	class ControllerDisconnectedEvent : public InputEvent {
	public:
		int Controller;

		ControllerDisconnectedEvent(int controller) : InputEvent(InputEventType::ControllerDisconnected), Controller(controller) {}
	};
}