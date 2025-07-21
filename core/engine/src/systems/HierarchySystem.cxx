#include "playground/systems/HierarchySystem.hxx"
#include "playground/ECS.hxx"
#include "playground/components/TranslationComponent.hxx"
#include "playground/components/RotationComponent.hxx"
#include "playground/components/ScaleComponent.hxx"
#include "playground/components/WorldTranslationComponent.hxx"
#include "playground/components/WorldRotationComponent.hxx"
#include "playground/components/WorldScaleComponent.hxx"
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
    void UpdateChildren(flecs::entity e, const WorldTranslationComponent& t, const WorldRotationComponent& r, const WorldScaleComponent& s) {
        // Update the world components based on the local components
        e.children([t, r, s](flecs::entity child) {
            child.set<WorldTranslationComponent>({ child.get<TranslationComponent>().position + t.position });
            child.set<WorldRotationComponent>({ child.get<RotationComponent>().rotation * r.rotation });
            child.set<WorldScaleComponent>({ child.get<ScaleComponent>().scale + s.scale });

            UpdateChildren(child, child.get<WorldTranslationComponent>(), child.get<WorldRotationComponent>(), child.get<WorldScaleComponent>());
        });
    }

    void Init(flecs::world world) {
        world.system<TranslationComponent, RotationComponent, ScaleComponent, WorldTranslationComponent, WorldRotationComponent, WorldScaleComponent>("HierarchySystem")
            .kind(flecs::PostUpdate)
            .multi_threaded(true)
            .without(flecs::ChildOf, flecs::Wildcard)
            .each([](
                flecs::entity e,
                TranslationComponent& t,
                RotationComponent& r,
                ScaleComponent& s,
                WorldTranslationComponent& wt,
                WorldRotationComponent& wr,
                WorldScaleComponent& ws
            ) {
                    ZoneScopedNC("HierarchySystem", tracy::Color::Green);
                    wt.position = t.position;
                    wr.rotation = r.rotation;
                    ws.scale = s.scale;
                UpdateChildren(e, wt, wr, ws);
            });
    }
}
