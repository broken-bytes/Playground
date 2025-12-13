#pragma once

#include <cstdint>

namespace playground::rendering {
    class GpuResource {
    public:
        virtual ~GpuResource() = default;

        static GpuResource* Create() { return new GpuResource(); }
    };
}
