#pragma once

#include "physics/PhysicsMaterialHandle.hxx"
#include <glm/glm.hpp>
#include <cstdint>

namespace playground::physics {
    void Init();
    void Update(double fixedDelta);
    void Shutdown();

    physics::PhysicsMaterialHandle CreateMaterial(float staticFriction, float dynamicFriction, float restitution);
    void AddBody(uint64_t entityId, float mass, float damping, glm::vec3 position, glm::vec4 rotation);
    void AddBoxCollider(uint64_t entityId, uint32_t materialId, glm::vec4 rotation, glm::vec3 dimensions, glm::vec3 offset);
    void AddShapeToBody(uint64_t body, uint64_t shape);
}
