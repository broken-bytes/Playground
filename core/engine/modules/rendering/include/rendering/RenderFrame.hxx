#pragma once

#include "rendering/VertexBufferHandle.hxx"
#include "rendering/IndexBufferHandle.hxx"
#include "rendering/MaterialHandle.hxx"
#include "rendering/DrawCall.hxx"
#include "rendering/Constants.hxx"
#include "rendering/CameraBuffer.hxx"
#include <array>


namespace playground::rendering {
    struct RenderFrame {
        bool isDirty;
        std::vector<DrawCall> drawCalls;
        std::array<CameraBuffer, MAX_CAMERA_COUNT> cameras;
    };
}
