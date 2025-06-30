#include "renderdoc_app.h"

#define WIN32_LEAN_AND_MEAN
#define NO_MINMAX
#include <Windows.h>
#include <cassert>
#include <filesystem>
#include <map>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL.h>
#include <playground/Engine.hxx>
#include <assetpipeline/assetpipeline.hxx>
#include <assetpipeline/loaders/ModelLoader.hxx>
#include <shared/Hardware.hxx>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_write.h>
#include <thread>

typedef void(__cdecl* Startup)(LookupTableDelegate, ScriptStartupCallback);

typedef void(__cdecl* CoreLayerStartUp)(PlaygroundConfig&);
typedef void(__cdecl* ScriptingLayerStartUp)(Startup);

CoreLayerStartUp coreStartup = nullptr;
uint16_t windowWidth = 1280;
uint16_t windowHeight = 720;
void* windowPtr = nullptr;

extern "C" uint8_t PlaygroundCoreMain(const PlaygroundConfig&);
extern "C" void PlaygroundMain(Startup start);

void StartUpEngine(LookupTableDelegate lookup, ScriptStartupCallback startup) {
    auto config = PlaygroundConfig{ windowPtr, lookup, windowWidth, windowHeight, false, startup };
    auto result = PlaygroundCoreMain(config);

    if (result == 2) {
        auto cpuName = playground::hardware::GetCPUBrandString();
        std::string message =
            "Your system does not meet the minimum requirements to run this game.\nAVX instructions are required.\n\n"
            + cpuName;
        SDL_ShowSimpleMessageBox(
            SDL_MESSAGEBOX_ERROR,
            "Unsupported CPU",
            message.c_str(),
            nullptr
        );
        SDL_Quit();
        std::exit(EXIT_FAILURE);
    }
}

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

    windowPtr = window;

    SDL_ShowWindow(window);

    auto props = SDL_GetWindowProperties(window);
#ifdef _WIN32
    windowPtr = (void*)SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
#endif

    PlaygroundMain([](auto lookup, auto startup) {
        StartUpEngine(lookup, startup);
    });

    SDL_HideWindow(window);

	return 0;
}
