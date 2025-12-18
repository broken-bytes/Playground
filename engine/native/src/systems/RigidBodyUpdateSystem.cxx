#include "playground/systems/RigidBodyUpdateSystem.hxx"
#include "playground/components/RigidBodyComponent.hxx"
#include "playground/components/WorldTransformComponent.hxx"
#include "playground/PhysicsManager.hxx"
#include <tracy/Tracy.hpp>

namespace playground::ecs::rigidbodyupdatesystem {
    void Init(flecs::world world) {
        world.system<RigidBodyComponent, const WorldTransformComponent>("RigidBodyUpdateSystem")
            .multi_threaded(true)
            .each([](flecs::entity e, RigidBodyComponent& rigidBody, const WorldTransformComponent& trans) {
                ZoneScopedNC("RigidBodyUpdateSystem", tracy::Color::Green);

                if (rigidBody.handle == UINT64_MAX) {
                    rigidBody.handle = physicsmanager::CreateRigidBody(
                        e.id(),
                        rigidBody.mass,
                        rigidBody.damping,
                        trans.Position,
                        trans.Rotation
                    );

                    return;
                }
                math::Vector3 position;
                math::Quaternion rotation;
                physicsmanager::GetBodyPosition(rigidBody.handle, &position);
                physicsmanager::GetBodyRotation(rigidBody.handle, &rotation);

                e.set<WorldTransformComponent>({ position, rotation, trans.Scale });
            });
    }
}
