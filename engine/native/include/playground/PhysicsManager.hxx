#pragma once

#include "playground/AssetManager.hxx"
#include <math/Vector3.hxx>
#include <math/Quaternion.hxx>
#include <cstdint>

namespace playground::physicsmanager {
    void Init();
    void Update(double delta);
    void Shutdown();
    uint64_t CreateRigidBody(uint64_t entityId, float mass, float damping, math::Vector3 position, math::Quaternion rotation);
    uint64_t CreateStaticBody(uint64_t entityId, math::Vector3 position, math::Quaternion rotation);
    uint64_t CreateBoxCollider(assetmanager::PhysicsMaterialHandle* materialHandle, math::Quaternion rotation, math::Vector3 dimensions, math::Vector3 offset, bool isTrigger);
    void AttachCollider(uint64_t body, uint64_t collider);
    void RemoveBody(uint64_t body);
    void RemoveCollider(uint64_t collider);

    void GetBodyPosition(uint64_t id, math::Vector3* position);
    void GetBodyRotation(uint64_t id, math::Quaternion* rotation);
}
