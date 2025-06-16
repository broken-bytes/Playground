#pragma once

#include "playground/AssetManager.hxx"
#include <glm/glm.hpp>
#include <cstdint>

namespace playground::drawcallbatcher {
    struct DrawCall {
        assetmanager::ModelHandle* modelHandle;
        uint16_t meshId;
        assetmanager::MaterialHandle* materialHandle;
        glm::mat4 transform;
    };

    void Batch(drawcallbatcher::DrawCall*, uint16_t count);
    void Submit();
}
