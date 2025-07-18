#include "playground/systems/StaticBodyUpdateSystem.hxx"
#include "playground/components/StaticBodyComponent.hxx"
#include "playground/components/WorldTranslationComponent.hxx"
#include "playground/components/WorldRotationComponent.hxx"
#include "playground/PhysicsManager.hxx"
#include <tracy/Tracy.hpp>

namespace playground::ecs::staticbodyupdatesystem {
    void Init(flecs::world world) {
        world.system<StaticBodyComponent, WorldTranslationComponent, WorldRotationComponent>("StaticBodyUpdateSystem")
            .kind(flecs::PostUpdate)
            .multi_threaded(true)
            .each([](flecs::entity e, StaticBodyComponent& rigidBody, WorldTranslationComponent& trans, WorldRotationComponent& rot) {
                ZoneScopedNC("StaticBodyUpdateSystem", tracy::Color::Green);

                if (rigidBody.handle == UINT64_MAX) {
                    rigidBody.handle = physicsmanager::CreateStaticBody(
                        e.id(),
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
