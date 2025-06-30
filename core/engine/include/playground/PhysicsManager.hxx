#pragma once

#include "playground/AssetManager.hxx"
#include <glm/glm.hpp>
#include <cstdint>

namespace playground::physicsmanager {
    void AddBody(uint64_t entityId, float mass, float damping, glm::vec3 position, glm::vec4 rotation);
    void AddBoxCollider(uint64_t entityId, assetmanager::PhysicsMaterialHandle* materialHandle, glm::vec4 rotation, glm::vec3 dimensions, glm::vec3 offset);
}
