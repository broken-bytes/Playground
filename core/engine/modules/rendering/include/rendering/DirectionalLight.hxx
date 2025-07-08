#pragma once

#include <math/Vector4.hxx>

namespace playground::rendering {
    struct DirectionalLight {
        math::Vector4 direction;
        math::Vector4 colour;
        float intensity;
    };
}
