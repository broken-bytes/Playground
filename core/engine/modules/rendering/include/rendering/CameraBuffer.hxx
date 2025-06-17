#pragma once

#include <glm/glm.hpp>

namespace playground::rendering {
    struct CameraBuffer {
        glm::mat4 ViewMatrix;
        glm::mat4 ProjectionMatrix;
    };
}
