#include "playground/systems/HierarchySystem.hxx"
#include "playground/ECS.hxx"
#include "playground/components/TransformComponent.hxx"
#include "playground/components/WorldTransformComponent.hxx"
#include "playground/components/MeshComponent.hxx"
#include "playground/components/MaterialComponent.hxx"
#include "playground/DrawCallBatcher.hxx"
#include <rendering/Constants.hxx>
#include <array>
#include <atomic>
#include <flecs.h>
#include <EASTL/fixed_vector.h>
#include <tracy/Tracy.hpp>
#include <shared/Arena.hxx>
#include <math/Math.hxx>
#include <memory>
#include <iostream>

namespace playground::ecs::hierarchysystem {
    void UpdateChildren(flecs::entity e, const WorldTransformComponent& t) {
        // Update the world components based on the local components
        e.children([t](flecs::entity child) {
            auto local = child.get<TransformComponent>();
            child.set<WorldTransformComponent>({
                t.Position + (t.Rotation * local.Position ),
                t.Rotation * local.Rotation,
                local.Scale * t.Scale
            });

            UpdateChildren(child, child.get<WorldTransformComponent>());
        });
    }

    void Init(flecs::world world) {
        world.system<TransformComponent, WorldTransformComponent>("HierarchySystem")
            .kind(flecs::PostUpdate)
            .multi_threaded(true)
            .without(flecs::ChildOf, flecs::Wildcard)
            .each([](
                flecs::entity e,
                TransformComponent& t,
                WorldTransformComponent& wt
            ) {
                    ZoneScopedNC("HierarchySystem", tracy::Color::Green);
                    wt.Position = t.Position;
                    wt.Rotation = t.Rotation;
                    wt.Scale = t.Scale;
                    UpdateChildren(e, wt);
            });
    }
}
