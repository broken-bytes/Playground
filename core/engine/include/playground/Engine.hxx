#pragma once

#include <cstdint>

extern "C" {
    typedef void (__cdecl *LookupTableDelegate)(const char*, void*);
    typedef void(__cdecl *ScriptStartupCallback)();

	struct {
		void* Window;
		LookupTableDelegate Delegate;
		uint32_t Width;
		uint32_t Height;
        bool IsOffscreen;
        ScriptStartupCallback startupCallback;

	} typedef PlaygroundConfig;

	__declspec(dllexport) uint8_t PlaygroundCoreMain(const PlaygroundConfig& config);
}
