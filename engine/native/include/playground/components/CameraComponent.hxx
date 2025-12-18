#pragma once

#include <cstdint>

struct CameraComponent {
    uint8_t Order;
    float Fov;
    float NearPlane;
    float FarPlane;
};
