#include "renderdoc_app.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <cassert>
#include <filesystem>
#include <SDL3/SDL.h>
#include <playground/Engine.hxx>
#include <assetpipeline/assetpipeline.hxx>
#include <assetpipeline/loaders/ModelLoader.hxx>
#include <assetdatabase/AssetDatabase.hxx>

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

	PlaygroundMain(PlaygroundConfig{
		.Window = nullptr,
		.Delegate = [](const char* name, void* ptr) {
			OutputDebugStringA(name);
		},
		.Width = 1280,
		.Height = 720
		}
	);

	return 0;
}
