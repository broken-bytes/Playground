#pragma once

#include <cstdint>

namespace playground {
    struct Transform {
        float position[3];
        float rotation[4];
        float scale[3];
    };

    struct MeshRenderer {
        uint32_t meshId;
        uint32_t materialId;
    };

    struct GameObject {
        uint32_t id;
        Transform transform;
    };
}
