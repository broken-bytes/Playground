#include "system/System.hxx"
#include <stdexcept>
#include <SDL3/SDL.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_platform.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
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
