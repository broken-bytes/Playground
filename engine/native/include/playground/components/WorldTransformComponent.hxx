#pragma once

#include "math/Vector3.hxx"
#include "math/Quaternion.hxx"

struct alignas(16) WorldTransformComponent {
    playground::math::Vector3 Position;
    playground::math::Quaternion Rotation;
    playground::math::Vector3 Scale;
};
