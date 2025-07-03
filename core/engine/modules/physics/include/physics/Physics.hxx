#pragma once

#include "physics/PhysicsMaterialHandle.hxx"
#include <glm/glm.hpp>
#include <cstdint>

namespace playground::physics {
    void Init();
    void Update(double fixedDelta);
    void Shutdown();

    physics::PhysicsMaterialHandle CreateMaterial(float staticFriction, float dynamicFriction, float restitution);
    uint64_t CreateRigidBody(float mass, float damping, glm::vec3 position, glm::vec4 rotation);
    uint64_t CreateStaticBody(glm::vec3 position, glm::vec4 rotation);
    uint64_t CreateBoxCollider(uint32_t materialId, glm::vec4 rotation, glm::vec3 dimensions, glm::vec3 offset);
    void AddShapeToBody(uint64_t body, uint64_t shape);
    void RemoveBody(uint64_t body);
    void RemoveShape(uint64_t shape);
}
