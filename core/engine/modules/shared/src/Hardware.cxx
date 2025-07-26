#include "shared/Hardware.hxx"

#if defined(_WIN32)
#include <intrin.h>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <timeapi.h>
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

    std::vector<CpuCore> GetCoresByEfficiency(CPUEfficiencyClass cpuClass) {
        DWORD len = 0;
        GetSystemCpuSetInformation(nullptr, 0, &len, GetCurrentProcess(), 0);

        std::vector<char> buffer(len);
        auto info = reinterpret_cast<SYSTEM_CPU_SET_INFORMATION*>(buffer.data());

        std::vector<CpuCore> cores;

        if (GetSystemCpuSetInformation(info, len, &len, GetCurrentProcess(), 0)) {
            char* ptr = buffer.data();
            while (ptr < buffer.data() + len) {
                auto* entry = reinterpret_cast<SYSTEM_CPU_SET_INFORMATION*>(ptr);

                if (entry->Type == CpuSetInformation) {
                    if (entry->CpuSet.Group == 0) {
                        bool isEfficient = entry->CpuSet.EfficiencyClass <= 0;

                        if (cpuClass == CPUEfficiencyClass::Efficient && isEfficient) {
                            cores.push_back({
                               entry->CpuSet.LogicalProcessorIndex,
                               entry->CpuSet.EfficiencyClass
                            });
                        } else if (cpuClass == CPUEfficiencyClass::Performance && !isEfficient) {
                            cores.push_back({
                               entry->CpuSet.LogicalProcessorIndex,
                               entry->CpuSet.EfficiencyClass
                            });
                        }
                    }
                }

                ptr += entry->Size;
            }
        }

        return cores;
    }

    void PinCurrentThreadToCore(uint32_t coreIndex) {
        GROUP_AFFINITY affinity = {};
        affinity.Group = 0;
        affinity.Mask = 1ull << coreIndex;

        if (!SetThreadGroupAffinity(GetCurrentThread(), &affinity, nullptr)) {
            exit(3);
        }

        SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

        PROCESS_POWER_THROTTLING_STATE state = {};
        state.Version = PROCESS_POWER_THROTTLING_CURRENT_VERSION;
        state.ControlMask = PROCESS_POWER_THROTTLING_EXECUTION_SPEED;
        state.StateMask = 0; // Disable throttling
        SetThreadInformation(GetCurrentThread(), ThreadPowerThrottling, &state, sizeof(state));
    }
}
