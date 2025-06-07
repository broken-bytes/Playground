#pragma once

#include <array>
#include <cstdint>

namespace playground {
    struct Transform {
        std::array<float, 3> position;
        std::array<float, 4> rotation;
        std::array<float, 3> scale;
    };

    struct MeshRenderer {
        uint32_t meshId;
        uint32_t materialId;
    };

    struct GameObject {
        uint32_t id;
        Transform transform;
    };


    namespace gameobjects {
        Transform* GetGameObjectTransform(uint32_t);
    }
}
