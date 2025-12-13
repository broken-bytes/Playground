#pragma once

#include <math/Vector3.hxx>
#include <cstdint>

struct AudioSourceComponent {
    playground::math::Vector3 previousPosition;
    playground::math::Vector3 forward;
    uint64_t handle = UINT64_MAX;
    const char* eventName;
};
