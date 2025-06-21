#pragma once

namespace playground::rendering {
    constexpr uint8_t MAX_CAMERA_COUNT = 16;
    constexpr uint16_t MAX_POINT_LIGHTS = 1024;

    constexpr uint8_t GLOBALS_BUFFER_BINDING = 0;
    constexpr uint8_t DIRECTIONAL_LIGHT_BUFFER_BINDING = 1;
    constexpr uint8_t CAMERA_BUFFER_BINDING = 2;
    constexpr uint8_t MATERIAL_BUFFER_BINDING = 3;

    constexpr uint8_t INSTANCE_BUFFER_BINDING = 0;

    constexpr uint8_t POINT_LIGHT_SRV_BINDING = 0;

    constexpr uint16_t MAX_BATCH_SIZE = 1024;
}
