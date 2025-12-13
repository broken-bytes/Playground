#pragma once

#include <math/Vector3.hxx>

namespace playground::rendering {
    struct PointLight {
        math::Vector3 position;
        math::Vector3 colour;
        float intensity;
        float radius;

        PointLight(math::Vector3 position, math::Vector3 colour, float intensity, float radius)
            : position(position), colour(colour), intensity(intensity), radius(radius) {
        }
    };
}
