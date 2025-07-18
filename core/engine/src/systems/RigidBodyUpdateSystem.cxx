#include "playground/systems/RigidBodyUpdateSystem.hxx"
#include "playground/components/RigidBodyComponent.hxx"
#include "playground/components/WorldTranslationComponent.hxx"
#include "playground/components/WorldRotationComponent.hxx"
#include "playground/PhysicsManager.hxx"
#include <tracy/Tracy.hpp>

namespace playground::ecs::rigidbodyupdatesystem {
    void Init(flecs::world world) {
        world.system<RigidBodyComponent, WorldTranslationComponent, WorldRotationComponent>("RigidBodyUpdateSystem")
            .kind(flecs::PostUpdate)
            .multi_threaded(true)
            .each([](flecs::entity e, RigidBodyComponent& rigidBody, WorldTranslationComponent& trans, WorldRotationComponent& rot) {
                ZoneScopedNC("RigidBodyUpdateSystem", tracy::Color::Green);

                if (rigidBody.handle == UINT64_MAX) {
                    rigidBody.handle = physicsmanager::CreateRigidBody(
                        e.id(),
                        rigidBody.mass,
                        rigidBody.damping,
                        trans.position,
                        rot.rotation
                    );

                    return;
                }
                physicsmanager::GetBodyPosition(rigidBody.handle, &trans.position);
                physicsmanager::GetBodyRotation(rigidBody.handle, &rot.rotation);
            });
    }
}
