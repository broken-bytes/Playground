#include "input/Input.hxx"
#include <events/Events.hxx>
#include <events/Event.hxx>
#include <events/InputEvent.hxx>
#include <events/SystemEvent.hxx>
#include <stdexcept>
#include <SDL3/SDL.h>
#include <iostream>

namespace playground::input {
	auto Init() -> void {
		if (!SDL_Init(SDL_INIT_EVENTS | SDL_INIT_GAMEPAD | SDL_INIT_HAPTIC)) {
			throw std::runtime_error("Failed to initialize SDL input");
		}
	}

    auto Shutdown() -> void {
    }

    auto Update() -> void {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_EVENT_KEY_DOWN:
            case SDL_EVENT_KEY_UP: {
                auto keyEvent = reinterpret_cast<SDL_KeyboardEvent*>(&event);

                break;
            }
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            case SDL_EVENT_MOUSE_BUTTON_UP: {
                auto mouseButtonEvent = reinterpret_cast<SDL_MouseButtonEvent*>(&event);

                break;
            }
            case SDL_EVENT_QUIT: {
                events::SystemEvent quitEvent(events::SystemEventType::Quit);
                playground::events::Emit(&quitEvent);
                break;
            }
            default:
                break;
            }
        }
	}
}
