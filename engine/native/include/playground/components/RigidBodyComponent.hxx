#pragma once

#include <cstdint>
#include <math/Vector3.hxx>

struct RigidBodyComponent {
    uint64_t handle = UINT64_MAX;
    float mass = 1.0f;
    float damping = 0.05f;
    bool isDirty = true;
};
