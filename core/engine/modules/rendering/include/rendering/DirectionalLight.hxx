#pragma once

#include <glm/glm.hpp>

namespace playground::rendering {
    struct DirectionalLight {
        glm::vec4 direction;
        glm::vec4 colour;
        float intensity;
    };
}
