#pragma once

#include <cstdint>

struct RigidBodyComponent {
    uint64_t handle = UINT64_MAX;
    float mass = 1.0f;
    float damping = 0.05f;
    bool isDirty = true;
};
