#include <cstdint>

extern "C" {
    typedef void (__cdecl *LookupTableDelegate)(const char*, void*);

	struct {
		void* Window;
		LookupTableDelegate Delegate;
		uint32_t Width;
		uint32_t Height;
        bool IsOffscreen;
	} typedef PlaygroundConfig;

	__declspec(dllexport) void PlaygroundMain(const PlaygroundConfig& config);
}
