#pragma once

#include "rendering/CommandQueue.hxx"
#include "rendering/CommandList.hxx"
#include "rendering/CommandListType.hxx"
#include "rendering/CommandAllocator.hxx"

#include <memory>

namespace playground::rendering {
    // Forward declaration
    class Device;
    class Context
    {
    public:
        virtual ~Context() = default;
        virtual auto Begin() -> void = 0;
        virtual auto Finish() -> void = 0;
    };
}
