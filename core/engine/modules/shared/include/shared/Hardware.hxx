#pragma once

#include <string>
#include <vector>

namespace playground::hardware {
    void Init();

    uint8_t CPUCount();

    bool SupportsAVX();

    bool SupportsAVX2();

    std::string GetCPUBrandString();

    struct CpuCore {
        uint32_t id;
        uint8_t efficiencyClass;
    };

    enum CPUEfficiencyClass {
        Efficient = 0,
        Performance = 1,
        Unknown = 2
    };

    std::vector<CpuCore> GetCoresByEfficiency(CPUEfficiencyClass);

    void PinCurrentThreadToCore(uint32_t coreIndex);
}
