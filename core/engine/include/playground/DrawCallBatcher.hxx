#pragma once

#include "playground/AssetManager.hxx"
#include <glm/glm.hpp>
#include <cstdint>

namespace playground::drawcallbatcher {
    struct DrawCall {
        assetmanager::ModelHandle* modelHandle;
        uint16_t meshId;
        assetmanager::MaterialHandle* materialHandle;
        glm::vec3 position;
        glm::vec4 rotation;
        glm::vec3 scale;
    };

    void Batch(drawcallbatcher::DrawCall*, uint16_t count);
    void Submit();
}
