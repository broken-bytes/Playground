#include "playground/PhysicsManager.hxx"
#include <physics/Physics.hxx>
#include <EASTL/hash_map.h>
#include <EASTL/vector.h>

namespace playground::physicsmanager {
    void AddBody(uint64_t entityId, float mass, float damping, glm::vec3 position, glm::vec4 rotation) {
        physics::AddBody(entityId, mass, damping, position, rotation);
    }

    void AddBoxCollider(uint64_t entityId, assetmanager::PhysicsMaterialHandle* materialHandle, glm::vec4 rotation, glm::vec3 dimensions, glm::vec3 offset) {
        physics::AddBoxCollider(entityId, materialHandle->material, rotation, dimensions, offset);
    }
}
