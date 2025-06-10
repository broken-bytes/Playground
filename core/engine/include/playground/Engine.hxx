#include <cstdint>

extern "C" {
    typedef void (__cdecl *LookupTableDelegate)(const char*, void*);
    typedef void(__cdecl *ScriptUpdateCallback)();

	struct {
		void* Window;
		LookupTableDelegate Delegate;
		uint32_t Width;
		uint32_t Height;
        bool IsOffscreen;
        ScriptUpdateCallback updateCallback;
	} typedef PlaygroundConfig;

	__declspec(dllexport) void PlaygroundCoreMain(const PlaygroundConfig& config);
}
