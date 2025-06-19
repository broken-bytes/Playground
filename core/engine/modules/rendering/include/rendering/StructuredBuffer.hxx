#pragma once

#include <cstdint>

namespace playground::rendering {
    class StructuredBuffer {
    public:
        StructuredBuffer() = default;
        virtual ~StructuredBuffer() = default;
        virtual auto SetData(const void* data, size_t count, size_t offset) -> void = 0;
    };
}
