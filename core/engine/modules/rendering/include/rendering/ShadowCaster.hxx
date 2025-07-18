#pragma once

#include "rendering/ShadowMap.hxx"
#include <math/Matrix4x4.hxx>
#include <memory>

namespace playground::rendering {
    struct ShadowCaster {
        math::Matrix4x4 ViewMatrix;
        math::Matrix4x4 ProjectionMatrix;
        uint32_t ShadowMapID;
    };
}
