#pragma once

#include "playground/AssetManager.hxx"
#include <math/Vector3.hxx>
#include <math/Vector4.hxx>
#include <math/Quaternion.hxx>
#include <math/Matrix4x4.hxx>
#include <cstdint>

namespace playground::drawcallbatcher {
    struct DrawCall {
        uint32_t modelHandle;
        uint16_t meshId;
        uint32_t materialHandle;
        math::Matrix4x4 transform;
    };

    void Batch(DrawCall*, uint16_t count);
    void SetSun(math::Vector3 direction, math::Vector4 colour, float intensity);
    void AddCamera(uint8_t order, float fov, float nearPlane, float farPlane, const math::Vector3& position, const math::Quaternion& rotation);
    void Submit();
}
