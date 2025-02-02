#include <cstdint>

extern "C" {
	typedef void (*LookupTableDelegate)(const char*, void*);

	struct {
		void* Window;
		LookupTableDelegate Delegate;
		uint32_t Width;
		uint32_t Height;
	} typedef PlaygroundConfig;

	__declspec(dllexport) void PlaygroundMain(const PlaygroundConfig& config);
}
