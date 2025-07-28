#include "system/System.hxx"
#include <SDL3/SDL.h>
#include <cstdint>
#include <string>
#include <stdexcept>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif

namespace playground::system {
	void* Init(uint16_t width, uint16_t height, bool fullscreen, std::string name) {
        auto flags = SDL_WINDOW_HIGH_PIXEL_DENSITY;
        if (fullscreen) {
            flags |= SDL_WINDOW_FULLSCREEN;
        }
        auto window = SDL_CreateWindow(name.c_str(), width, height, flags);

        auto result = SDL_ShowWindow(window);
        if (result == false) {
            throw std::runtime_error("Failed to show window: " + std::string(SDL_GetError()));
        }

        auto props = SDL_GetWindowProperties(window);
#ifdef _WIN32
        return (void*)SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
#endif
	}
}
