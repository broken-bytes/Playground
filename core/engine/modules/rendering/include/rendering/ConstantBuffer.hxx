#pragma once

#include <cstdint>

namespace playground::rendering {
    class ConstantBuffer {
    public:
        ConstantBuffer() = default;
        virtual ~ConstantBuffer() = default;
        virtual auto Update(void* data, size_t size) -> void = 0;
    };
}
