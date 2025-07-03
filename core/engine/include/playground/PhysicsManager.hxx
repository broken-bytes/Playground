#pragma once

#include "playground/AssetManager.hxx"
#include <glm/glm.hpp>
#include <cstdint>

namespace playground::physicsmanager {
    void Init();
    void Update(double delta);
    void Shutdown();
    uint64_t CreateRigidBody(float mass, float damping, glm::vec3 position, glm::vec4 rotation);
    uint64_t CreateStaticBody(glm::vec3 position, glm::vec4 rotation);
    uint64_t CreateBoxCollider(assetmanager::PhysicsMaterialHandle* materialHandle, glm::vec4 rotation, glm::vec3 dimensions, glm::vec3 offset);
    void AttachCollider(uint64_t body, uint64_t collider);
    void RemoveBody(uint64_t body);
    void RemoveCollider(uint64_t collider);
}
