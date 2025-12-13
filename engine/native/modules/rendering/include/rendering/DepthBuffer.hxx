#pragma once

#include "GpuResource.hxx"

namespace playground::rendering {
    class DepthBuffer : public GpuResource {
    public:
        explicit DepthBuffer() : GpuResource() {}

        virtual ~DepthBuffer() = default;
    };
}
