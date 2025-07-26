#include "playground/systems/RigidBodyUpdateSystem.hxx"
#include "playground/components/RigidBodyComponent.hxx"
#include "playground/components/TranslationComponent.hxx"
#include "playground/components/RotationComponent.hxx"
#include "playground/PhysicsManager.hxx"
#include <tracy/Tracy.hpp>

namespace playground::ecs::rigidbodyupdatesystem {
    void Init(flecs::world world) {
        world.system<RigidBodyComponent, const TranslationComponent, const RotationComponent>("RigidBodyUpdateSystem")
            .multi_threaded(true)
            .each([](flecs::entity e, RigidBodyComponent& rigidBody, const TranslationComponent& trans, const RotationComponent& rot) {
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
                math::Vector3 position;
                math::Quaternion rotation;
                physicsmanager::GetBodyPosition(rigidBody.handle, &position);
                physicsmanager::GetBodyRotation(rigidBody.handle, &rotation);

                e.set<TranslationComponent>({ true, position });
                e.set<RotationComponent>({ rotation });
            });
    }
}
