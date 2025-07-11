#pragma once

#include <math/Vector4.hxx>
#include <math/Matrix4x4.hxx>

namespace playground::rendering {
    struct DirectionalLight {
        math::Matrix4x4 viewProj;
        math::Vector4 direction;
        math::Vector4 colour;
    };
}
