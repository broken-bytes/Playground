#pragma once

#include "rendering/VertexBufferHandle.hxx"
#include "rendering/IndexBufferHandle.hxx"
#include "rendering/MaterialHandle.hxx"
#include "rendering/DrawCall.hxx"
#include "rendering/Constants.hxx"
#include "rendering/Camera.hxx"
#include "rendering/DirectionalLight.hxx"
#include <array>
#include <vector>
#include <EASTL/array.h>
#include <EASTL/vector.h>
#include <shared/Memory.hxx>


namespace playground::rendering {
    struct RenderFrame {
        bool isDirty;
        eastl::vector<DrawCall> drawCalls;
        eastl::vector<Camera> cameras;
        DirectionalLight sun;
    };
}
