#include <cassert>
#include <filesystem>
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
    auto config = PlaygroundConfig{
        lookup,
#if EDITOR
        nullptr,
#endif,
        startup,
        windowWidth,
        windowHeight,
        true,
        "Test",
        workDir.c_str(),
        nullptr
    };

    PlaygroundCoreMain(config);
}

void StartupDelegate(
    LookupTableDelegate lookupPtr,
    ScriptStartupCallback startupPtr
) {
    StartUpEngine(lookupPtr, startupPtr);
}

int main(int argc, char** argv) {
    LookupTableDelegate lookupPtr;
    ScriptStartupCallback startupPtr;

    PlaygroundMain(StartupDelegate);


	return 0;
}
