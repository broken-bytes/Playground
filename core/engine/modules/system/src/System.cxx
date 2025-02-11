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

        return windowHandle;
	}
}
