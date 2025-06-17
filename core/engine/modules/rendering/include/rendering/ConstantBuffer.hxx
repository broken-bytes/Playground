#pragma once

#include <cstdint>

namespace playground::rendering {
    class ConstantBuffer {
    public:
        ConstantBuffer() = default;
        virtual ~ConstantBuffer() = default;
        virtual auto SetData(const void* data, size_t count, size_t offset) -> void = 0;
    };
}
