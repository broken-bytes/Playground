#include "renderdoc_app.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <cassert>
#include <filesystem>
#include <map>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL.h>
#include <playground/Engine.hxx>
#include <assetpipeline/assetpipeline.hxx>
#include <assetpipeline/loaders/ModelLoader.hxx>
#include <assetdatabase/AssetDatabase.hxx>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_write.h>

typedef void(__cdecl* ScriptingLayerStartUp)(void*, uint32_t, uint32_t, bool);

int SDL_main(int argc, char** argv) {
	RENDERDOC_API_1_1_2* rdoc_api = nullptr;

	// At init, on windows
	if (HMODULE mod = GetModuleHandleA("renderdoc.dll"))
	{
		auto RENDERDOC_GetAPI =
			(pRENDERDOC_GetAPI)GetProcAddress(mod, "RENDERDOC_GetAPI");
		int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_1_2, reinterpret_cast<void**>(&rdoc_api));
		assert(ret == 1);
	}

    auto window = SDL_CreateWindow("Playground", 1280, 720, SDL_WINDOW_HIGH_PIXEL_DENSITY);

    SDL_ShowWindow(window);

    // Get the native window handle
    auto props = SDL_GetWindowProperties(window);
    void* ptr = nullptr;
#ifdef _WIN32
    ptr = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
#endif

    // Load the Swift library
#if _WIN32
    auto lib = LoadLibrary("Playground.dll");

    if (lib == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load Playground.dll: %s", GetLastError());
        return -1;
    }

    auto startUp = (ScriptingLayerStartUp)GetProcAddress(lib, "PlaygroundMain");

    startUp(ptr, 1280, 720, true);
#endif

    SDL_HideWindow(window);

	return 0;
}
