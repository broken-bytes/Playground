#include "renderdoc_app.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <cassert>
#include <filesystem>
#include <map>
#include <SDL3/SDL.h>
#include <playground/Engine.hxx>
#include <assetpipeline/assetpipeline.hxx>
#include <assetpipeline/loaders/ModelLoader.hxx>
#include <assetdatabase/AssetDatabase.hxx>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_write.h>

std::map<std::string, void*> functions = std::map<std::string, void*>();

typedef void (*RenderingPreFrameFunction)();
typedef void (*RenderingUpdateFunction)(double);
typedef void (*RenderingPostFrameFunction)();
typedef void (*RenderingReadBackBuffer)(void* data, size_t* numBytes);

int main() {
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

	PlaygroundMain(PlaygroundConfig{
		.Window = ptr,
		.Delegate = [](const char* name, void* ptr) {
            functions.insert({ name, ptr });
		},
		.Width = 1280,
		.Height = 720,
        .IsOffscreen = true
		}
	);

    RenderingPreFrameFunction preFrameFunction = reinterpret_cast<RenderingPreFrameFunction>(functions["Rendering_PreFrame"]);
    RenderingUpdateFunction updateFunction = reinterpret_cast<RenderingUpdateFunction>(functions["Rendering_Update"]);
    RenderingPostFrameFunction postFrameFunction = reinterpret_cast<RenderingPostFrameFunction>(functions["Rendering_PostFrame"]);
    RenderingReadBackBuffer readBackBuffer = reinterpret_cast<RenderingReadBackBuffer>(functions["Rendering_ReadBackBuffer"]);

    auto now = std::chrono::high_resolution_clock::now();
    double deltaTime = 0.0;

    bool quit = false;

    uint8_t* data = new uint8_t[1280 * 720 * 4];
    size_t numBytes = 0;

    while (!quit) {

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
            {
                quit = true;
            }
        }

        preFrameFunction();
        updateFunction(deltaTime);
        postFrameFunction();

        readBackBuffer(reinterpret_cast<void*>(data), &numBytes);

        stbi_write_jpg("C:\\Users\\marce\\Desktop\\ReadbackBuffer.jpg", 1280, 720, 4, data, 90);

        auto end = std::chrono::high_resolution_clock::now();
        deltaTime = std::chrono::duration<double>(end - now).count();
    }

	return 0;
}
