#include <cassert>
#include <filesystem>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL.h>
#include <playground/Engine.hxx>

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
extern "C" void AssemblyMain();

void StartUpEngine(LookupTableDelegate lookup, ScriptStartupCallback startup) {
    auto workDir = std::filesystem::current_path().string();
    auto config = PlaygroundConfig{ lookup, startup, windowWidth, windowHeight, true, "Test", workDir.c_str()};

    PlaygroundCoreMain(config);
}

int SDL_main(int argc, char** argv) {
    PlaygroundMain([](auto lookup, auto startup) {
        StartUpEngine(lookup, startup);
    });

	return 0;
}
