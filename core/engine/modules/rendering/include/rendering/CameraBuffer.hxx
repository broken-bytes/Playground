#pragma once

#include <math/Matrix4x4.hxx>

namespace playground::rendering {
    struct CameraBuffer {
        math::Matrix4x4 ViewMatrix;
        math::Matrix4x4 ProjectionMatrix;
    };
}
