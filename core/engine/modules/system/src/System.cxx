#include "system/System.hxx"
#include <stdexcept>
#include <SDL3/SDL.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_platform.h>

#ifdef _WIN32
#include <Windows.h>
#endif

namespace playground::system {
	auto Init(void* windowHandle) -> void* {
		if (not SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
			throw std::runtime_error("Failed to initialize SDL3");
		}

        SDL_PropertiesID props = SDL_CreateProperties();
        SDL_SetPointerProperty(props, SDL_PROP_WINDOW_CREATE_WIN32_HWND_POINTER, windowHandle);
        SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_HIDDEN_BOOLEAN, true);

        SDL_CreateWindowWithProperties(props);

        return windowHandle;
	}
}
