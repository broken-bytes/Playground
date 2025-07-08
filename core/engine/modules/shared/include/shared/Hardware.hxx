#pragma once

#include <string>

namespace playground::hardware {
    void Init();

    uint8_t CPUCount();

    bool SupportsAVX();

    bool SupportsAVX2();

    std::string GetCPUBrandString();
}
