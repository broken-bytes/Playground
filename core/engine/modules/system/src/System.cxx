#include "system/System.hxx"
#include <stdexcept>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif

namespace playground::system {
	auto Init(void* windowHandle) -> void* {
        return windowHandle;
	}
}
