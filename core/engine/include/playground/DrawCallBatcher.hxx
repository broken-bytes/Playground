#pragma once

#include "playground/AssetManager.hxx"
#include <math/Vector3.hxx>
#include <math/Vector4.hxx>
#include <math/Quaternion.hxx>
#include <math/Matrix4x4.hxx>
#include <cstdint>

namespace playground::drawcallbatcher {
    struct DrawCall {
        assetmanager::ModelHandle* modelHandle;
        uint16_t meshId;
        assetmanager::MaterialHandle* materialHandle;
        math::Matrix4x4 transform;
    };

    void Batch(drawcallbatcher::DrawCall*, uint16_t count);
    void SetSun(math::Vector3 direction, math::Vector4 colour, float intensity);
    void AddCamera(uint8_t order, float fov, float nearPlane, float farPlane, math::Vector3& position, math::Quaternion& rotation);
    void Submit();
}
