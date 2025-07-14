#pragma once

#include <cstdint>

namespace playground::rendering {
    class Cubemap {
    public:
        virtual ~Cubemap() = default;

        virtual uint32_t ID() const = 0;
    };
}

