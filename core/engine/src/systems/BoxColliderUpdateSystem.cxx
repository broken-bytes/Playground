#pragma once

#include "playground/components/RigidBodyComponent.hxx"
#include "playground/components/StaticBodyComponent.hxx"
#include "playground/components/BoxColliderComponent.hxx"
#include "playground/PhysicsManager.hxx"
#include <flecs.h>
#include <tracy/Tracy.hpp>

namespace playground::ecs::boxcolliderupdatesystem {

    void Init(flecs::world world) {
        world.system<BoxColliderComponent>("BoxColliderUpdateSystem")
            .kind(flecs::PostUpdate)
            .multi_threaded(true)
            .each([](flecs::entity e, BoxColliderComponent& boxCollider) {
                ZoneScopedNC("BoxColliderUpdateSystem", tracy::Color::Green);
                // The collider is invalid if it has no handle
                if (boxCollider.handle == UINT64_MAX) {
                    boxCollider.handle = physicsmanager::CreateBoxCollider(
                        boxCollider.material,
                        boxCollider.rotation,
                        boxCollider.dimensions,
                        boxCollider.offset,
                        boxCollider.isTrigger
                    );

                    return;
                }

                // If the collider is already attached to a body, we do not want to reattach it
                if (boxCollider.bodyHandle != UINT64_MAX) {
                    return;
                }

                flecs::entity target = e;

                while (target.is_valid()) {
                    if (e.has<RigidBodyComponent>()) {
                        auto body = e.get<RigidBodyComponent>();
                        if (body.handle == UINT64_MAX) {
                            return; // The body is invalid, do not attach the collider
                        }

                        boxCollider.bodyHandle = body.handle;
                        physicsmanager::AttachCollider(body.handle, boxCollider.handle);

                        break;
                    }
                    else if (e.has<StaticBodyComponent>()) {
                        auto body = e.get<StaticBodyComponent>();
                        if (body.handle == UINT64_MAX) {
                            return; // The static body is invalid, do not attach the collider
                        }

                        boxCollider.bodyHandle = body.handle;
                        physicsmanager::AttachCollider(body.handle, boxCollider.handle);

                        break;
                    }
                    else {
                        target = target.parent();
                    }
                }
            });
    }
}
