#include "system/System.hxx"
#include <stdexcept>
#include <SDL3/SDL.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_platform.h>

namespace playground::system {
	auto Init(void* windowHandle) -> void* {
		if (not SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
			throw std::runtime_error("Failed to initialize SDL3");
		}

		if (windowHandle != nullptr) {
			SDL_PropertiesID props = {};
			SDL_SetPointerProperty(props, SDL_PROP_WINDOW_CREATE_WIN32_HWND_POINTER, windowHandle);
		}
		else {
			SDL_Window* window = SDL_CreateWindow("Foo", 1280, 720, 0);
			if (not window) {
				throw std::runtime_error("Failed to create window");
			}

			SDL_ShowWindow(window);

			return window;
		}
	}
}
