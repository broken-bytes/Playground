#pragma once

#include <memory>
#include <wrl.h>
#include "rendering/CPUResourceHandle.hxx"
#include "rendering/GPUResourceHandle.hxx"

namespace playground::rendering
{
    class Heap
    {
        public:
        virtual ~Heap() = 0;

        [[nodiscard]]
        virtual auto Size()->size_t = 0;

        [[nodiscard]]
        virtual auto Increment()->size_t = 0;

        [[nodiscard]]
        virtual auto CPUHandleForHeapStart() -> std::shared_ptr<CPUResourceHandle> = 0;

        [[nodiscard]]
        virtual auto GPUHandleForHeapStart() -> std::shared_ptr<GPUResourceHandle> = 0;

        [[nodiscard]]
        virtual auto CpuHandleFor(std::uint32_t index) -> std::shared_ptr<CPUResourceHandle> = 0;

        [[nodiscard]]
        virtual auto GpuHandleFor(std::uint32_t index) -> std::shared_ptr<GPUResourceHandle> = 0;

        [[nodiscard]]
        virtual auto IsFilled() -> bool = 0;
    };
}
