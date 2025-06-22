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
    void SetSun(glm::vec3 direction, glm::vec4 colour, float intensity);
    void AddCamera(uint8_t order, float fov, float nearPlane, float farPlane, glm::vec3& position, glm::quat& rotation);
    void Submit();
}
