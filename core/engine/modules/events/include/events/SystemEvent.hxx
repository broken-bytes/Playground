#pragma once

#include "events/Event.hxx"

namespace playground::events {
	enum class SystemEventType: uint32_t {
		Quit = 0,
		WindowResize,
		WindowMove,
		WindowFocus,
		WindowLostFocus,
	};

	class SystemEvent : public Event {
	public:
		SystemEventType SystemType;

		SystemEvent(SystemEventType systemType) : Event(EventType::System), SystemType(systemType) {}
	};

	class QuitEvent : public SystemEvent {
	public:
		QuitEvent() : SystemEvent(SystemEventType::Quit) {}
	};

	class WindowResizeEvent : public SystemEvent {
	public:
		int Width;
		int Height;

		WindowResizeEvent(int width, int height) : SystemEvent(SystemEventType::WindowResize), Width(width), Height(height) {}
	};

	class WindowMoveEvent : public SystemEvent {
	public:
		int X;
		int Y;

		WindowMoveEvent(int x, int y) : SystemEvent(SystemEventType::WindowMove), X(x), Y(y) {}
	};

	class WindowFocusEvent : public SystemEvent {
	public:
		WindowFocusEvent() : SystemEvent(SystemEventType::WindowFocus) {}
	};

	class WindowLostFocusEvent : public SystemEvent {
	public:
		WindowLostFocusEvent() : SystemEvent(SystemEventType::WindowLostFocus) {}
	};
}
