#pragma once

#include <math/Vector3.hxx>
#include <math/Quaternion.hxx>
#include "playground/AssetManager.hxx"

#pragma pack(push, 8)
struct BoxColliderComponent {
    playground::math::Vector3 dimensions{ 1.0f, 1.0f, 1.0f };
    playground::math::Vector3 offset{ 0.0f, 0.0f, 0.0f };
    playground::math::Quaternion rotation{ 0.0f, 0.0f, 0.0f, 1.0f };
    playground::assetmanager::PhysicsMaterialHandle* material = nullptr;
    uint64_t handle = UINT64_MAX;
    uint64_t bodyHandle = UINT64_MAX;
    bool isTrigger = false;
};
#pragma pack(pop)
