#include "playground/systems/StaticBodyUpdateSystem.hxx"
#include "playground/components/StaticBodyComponent.hxx"
#include "playground/components/TransformComponent.hxx"
#include "playground/PhysicsManager.hxx"
#include <tracy/Tracy.hpp>

namespace playground::ecs::staticbodyupdatesystem {
    void Init(flecs::world world) {
        world.system<StaticBodyComponent, const TransformComponent>("StaticBodyUpdateSystem")
            .kind(flecs::PostUpdate)
            .multi_threaded(true)
            .each([](flecs::entity e, StaticBodyComponent& rigidBody, const TransformComponent& trans) {
                ZoneScopedNC("StaticBodyUpdateSystem", tracy::Color::Green);

                if (rigidBody.handle == UINT64_MAX) {
                    rigidBody.handle = physicsmanager::CreateStaticBody(
                        e.id(),
                        trans.Position,
                        trans.Rotation
                    );

                    return;
                }

                math::Vector3 position;
                math::Quaternion rotation;
                physicsmanager::GetBodyPosition(rigidBody.handle, &position);
                physicsmanager::GetBodyRotation(rigidBody.handle, &rotation);

                e.set<TransformComponent>({ position, rotation, trans.Scale });
            });
    }
}
