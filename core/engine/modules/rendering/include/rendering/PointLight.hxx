#pragma once

#include <glm/glm.hpp>

namespace playground::rendering {
    struct PointLight {
        glm::vec3 position;
        glm::vec3 colour;
        float intensity;
        float radius;

        PointLight(glm::vec3 position, glm::vec3 colour, float intensity, float radius)
            : position(position), colour(colour), intensity(intensity), radius(radius) {
        }
    };
}
