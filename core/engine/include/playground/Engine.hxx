#pragma once

#include <cstdint>

extern "C" {
    typedef void (__cdecl *LookupTableDelegate)(const char*, void*);
    typedef void(__cdecl *ScriptStartupCallback)();

	struct {
		LookupTableDelegate Delegate;
#if EDITOR
        LookupTableDelegate EditorDelegate;
#endif
        ScriptStartupCallback startupCallback;
		uint32_t Width;
		uint32_t Height;
        bool Fullscreen;
        const char* Name;
        const char* Path;
        void* WindowHandle;

	} typedef PlaygroundConfig;

	__declspec(dllexport) uint8_t PlaygroundCoreMain(const PlaygroundConfig& config);
}
