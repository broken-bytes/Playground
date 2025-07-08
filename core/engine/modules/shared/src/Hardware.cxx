#include "shared/Hardware.hxx"

#if defined(_WIN32)
#include <intrin.h>
#elif defined(__GNUC__) || defined(__clang__)
#include <cpuid.h>
#endif

#include <array>
#include <thread>

namespace playground::hardware {
    bool supportAVX = false;
    bool supportAVX2 = false;

    void Init() {
#if defined(_WIN32)
        int regs[4];
        __cpuid(regs, 1);

        bool osXSAVE = (regs[2] & (1 << 27)) != 0;
        bool cpuAVX = (regs[2] & (1 << 28)) != 0;

        if (osXSAVE && cpuAVX) {
            unsigned long long xcr = _xgetbv(0);
            supportAVX = (xcr & 0x6) == 0x6;
        }

        __cpuid(regs, 0);
        if (regs[0] >= 7) {
            __cpuidex(regs, 7, 0);
            supportAVX2 = (regs[1] & (1 << 5)) != 0 && supportAVX;
        }
#elif defined(__GNUC__) || defined(__clang__)
        supportAVX = true;    // Assume for now
        supportAVX2 = true;
#else
        supportAVX = true;
        supportAVX2 = false;
#endif
    }

    uint8_t CPUCount() {
        return std::thread::hardware_concurrency();
    }

    bool SupportsAVX() {
        return supportAVX;
    }

    bool SupportsAVX2() {
        return supportAVX2;
    }

    std::string GetCPUBrandString() {
        std::array<int, 4> cpui;
        std::string result;

        for (int i = 0x80000002; i <= 0x80000004; i++) {
            __cpuid(cpui.data(), i);
            result.append(reinterpret_cast<char*>(cpui.data()), sizeof(cpui));
        }

        return result;
    }
}
