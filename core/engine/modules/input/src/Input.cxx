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

	auto Update() -> void {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            std::cout << "Event: " << event.type << "|" << event.key.scancode << std::endl;
        }
	}
}
