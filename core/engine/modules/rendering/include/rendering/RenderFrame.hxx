#pragma once

#include "rendering/VertexBufferHandle.hxx"
#include "rendering/IndexBufferHandle.hxx"
#include "rendering/MaterialHandle.hxx"
#include "rendering/DrawCall.hxx"
#include <vector>

namespace playground::rendering {
    struct RenderFrame {
        bool isValid = false;
        std::vector<DrawCall> drawCalls;
    };
}
