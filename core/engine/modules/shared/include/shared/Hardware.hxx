#pragma once

#include <string>

namespace playground::hardware {
    void Init();

    bool SupportsAVX();

    bool SupportsAVX2();

    std::string GetCPUBrandString();
}
