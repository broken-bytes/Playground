#pragma once

namespace playground::rendering {
    constexpr uint8_t MAX_CAMERA_COUNT = 16;
    constexpr uint16_t MAX_POINT_LIGHTS = 1024;

    constexpr uint16_t MAX_SRV_HEAP_SIZE = 8192;
    constexpr uint16_t MAX_BATCH_SIZE = 1024;
    constexpr uint32_t MAX_DRAW_CALLS_PER_FRAME = 131072;

    // Root Signature Bindings Post Skybox Shaders
    constexpr uint8_t SB_BINDLESS_CUBEMAPS_SLOT = 6;
    constexpr uint8_t SB_BINDLESS_SHADOW_MAPS_SLOT = 7;
    constexpr uint8_t SB_SHADOW_CASTERS_BUFFER_BINDING = 8;
    constexpr uint8_t SB_POINT_LIGHT_SRV_BINDING = 0;

    // Shared Root Signature Bindings
    constexpr uint8_t GLOBALS_BUFFER_BINDING = 0;
    constexpr uint8_t CAMERA_BUFFER_BINDING = 2;
    constexpr uint8_t MATERIAL_BUFFER_BINDING = 3;

    // Root Signature Bindings Standard Shaders
    constexpr uint8_t DIRECTIONAL_LIGHT_BUFFER_BINDING = 1;
    constexpr uint8_t SHADOW_CASTERS_COUNT_BINDING = 4;
    constexpr uint8_t BINDLESS_TEXTURES_SLOT = 5;
    constexpr uint8_t BINDLESS_CUBEMAPS_SLOT = 6;
    constexpr uint8_t BINDLESS_SHADOW_MAPS_SLOT = 7;
    constexpr uint8_t SHADOW_CASTERS_BUFFER_BINDING = 8;
    constexpr uint8_t POINT_LIGHT_SRV_BINDING = 0;

    // Root Signature Bindings Post Process Shaders
    constexpr uint8_t PP_INVERSE_SCREEN_RES_CONSTANTS_BINDING = 1;
    constexpr uint8_t PP_FRAME_COLOUR_BUFFER_BINDING = 4;
    constexpr uint8_t PP_FRAME_DEPTH_BUFFER_BINDING = 5;
    constexpr uint8_t PP_BINDLESS_TEXTURES_SLOT = 6;
    constexpr uint8_t PP_BINDLESS_CUBEMAPS_SLOT = 7;

    // Shadow constants
    constexpr uint16_t MAX_SHADOW_RES_DIRECTIONAL_LIGHT = 4096;
    constexpr uint16_t MAX_SHADOW_RES_POINT_AND_SPOT_LIGHT = 1024;
    constexpr uint8_t MAX_SHADOW_MAPS_PER_FRAME = 32;
    constexpr uint16_t SUN_SIZE = 500;
    constexpr float SUN_NEAR_PLANE = 0.1f;
    constexpr float SUN_FAR_PLANE = 500;
}
