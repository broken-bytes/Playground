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

    uint64_t CreateRigidBody(uint64_t entityId, float mass, float damping, math::Vector3 position, math::Quaternion rotation) {
        return physics::CreateRigidBody(entityId, mass, damping, position, rotation);
    }

    uint64_t CreateStaticBody(uint64_t entityId, math::Vector3 position, math::Quaternion rotation) {
        return physics::CreateStaticBody(entityId, position, rotation);
    }

    uint64_t CreateBoxCollider(assetmanager::PhysicsMaterialHandle* materialHandle, math::Quaternion rotation, math::Vector3 dimensions, math::Vector3 offset, bool isTrigger) {
        return physics::CreateBoxCollider(materialHandle->material, rotation, dimensions, offset, isTrigger);
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

    void GetBodyPosition(uint64_t id, math::Vector3* position) {
        physics::GetBodyPosition(id, position);
    }

    void GetBodyRotation(uint64_t id, math::Quaternion* rotation) {
        physics::GetBodyRotation(id, rotation);
    }
}
