#include "playground/PhysicsManager.hxx"
#include <physics/Physics.hxx>
#include <EASTL/hash_map.h>
#include <EASTL/vector.h>

namespace playground::physicsmanager {
    void Init() {
        physics::Init();
    }

    void Update(double delta) {
        if (delta < 0.001) {
            return;
        }

        physics::Update(delta);
    }

    void Shutdown() {
        physics::Shutdown();
    }

    uint64_t CreateRigidBody(float mass, float damping, glm::vec3 position, glm::vec4 rotation) {
        return physics::CreateRigidBody(mass, damping, position, rotation);
    }

    uint64_t CreateStaticBody(glm::vec3 position, glm::vec4 rotation) {
        return physics::CreateStaticBody(position, rotation);
    }

    uint64_t CreateBoxCollider(assetmanager::PhysicsMaterialHandle* materialHandle, glm::vec4 rotation, glm::vec3 dimensions, glm::vec3 offset) {
        return physics::CreateBoxCollider(materialHandle->material, rotation, dimensions, offset);
    }

    void AttachCollider(uint64_t body, uint64_t collider) {
        physics::AddShapeToBody(body, collider);
    }

    void RemoveBody(uint64_t body) {
        physics::RemoveBody(body);
    }

    void RemoveCollider(uint64_t collider) {
        physics::RemoveShape(collider);
    }
}
