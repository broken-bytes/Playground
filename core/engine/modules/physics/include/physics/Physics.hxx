#pragma once

#include "physics/PhysicsMaterialHandle.hxx"
#include <math/Vector3.hxx>
#include <math/Quaternion.hxx>
#include <cstdint>

namespace playground::physics {
    void Init();
    void Update(double fixedDelta);
    void Shutdown();

    physics::PhysicsMaterialHandle CreateMaterial(float staticFriction, float dynamicFriction, float restitution);
    uint64_t CreateRigidBody(float mass, float damping, math::Vector3 position, math::Quaternion rotation);
    uint64_t CreateStaticBody(math::Vector3 position, math::Quaternion rotation);
    uint64_t CreateBoxCollider(uint32_t materialId, math::Quaternion rotation, math::Vector3 dimensions, math::Vector3 offset);
    void AddShapeToBody(uint64_t body, uint64_t shape);
    void RemoveBody(uint64_t body);
    void RemoveShape(uint64_t shape);

    void GetBodyPosition(uint64_t id, math::Vector3* position);
    void GetBodyRotation(uint64_t id, math::Quaternion* rotation);
}
