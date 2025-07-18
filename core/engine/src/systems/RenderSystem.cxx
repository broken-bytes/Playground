#include "playground/systems/RenderSystem.hxx"
#include "playground/ECS.hxx"
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

namespace playground::ecs::rendersystem {
    auto drawCalls = new drawcallbatcher::DrawCall[rendering::MAX_DRAW_CALLS_PER_FRAME];
    math::Matrix4x4 matrices[rendering::MAX_DRAW_CALLS_PER_FRAME];

    std::atomic<uint32_t> offset = 0;

    void Init(flecs::world world) {
        auto translation = ecs::RegisterComponent("WorldTranslationComponent", sizeof(WorldTranslationComponent), alignof(WorldTranslationComponent));
        auto rotation = ecs::RegisterComponent("WorldRotationComponent", sizeof(WorldRotationComponent), alignof(WorldRotationComponent));
        auto scale = ecs::RegisterComponent("WorldScaleComponent", sizeof(WorldScaleComponent), alignof(WorldScaleComponent));
        auto mesh = ecs::RegisterComponent("MeshComponent", sizeof(MeshComponent), alignof(MeshComponent));
        auto material = ecs::RegisterComponent("MaterialComponent", sizeof(MaterialComponent), alignof(MaterialComponent));

        std::array<uint64_t, 5> components = {
            translation,
            rotation,
            scale,
            mesh,
            material
        };

        ecs::CreatePostUpdateSystem("RenderSystem", components.data(), components.size(), true,
            [](ecs_iter_t* iter) {
                ZoneScopedNC("RenderSystem", tracy::Color::Green);
                auto translation = ecs_field(iter, WorldTranslationComponent, 0);
                auto rotation = ecs_field(iter, WorldRotationComponent, 1);
                auto scale = ecs_field(iter, WorldScaleComponent, 2);
                auto mesh = ecs_field(iter, MeshComponent, 3);
                auto material = ecs_field(iter, MaterialComponent, 4);

                auto startIndex = offset.fetch_add(iter->count, std::memory_order_relaxed);

                math::Mat4FromPRSBulk(
                    reinterpret_cast<math::Vector3*>(translation),
                    reinterpret_cast<math::Quaternion*>(rotation),
                    reinterpret_cast<math::Vector3*>(scale),
                    iter->count,
                    &matrices[startIndex]
                );
                auto drawPtr = &drawCalls[startIndex];
                for (int x = 0; x < iter->count; x++) {
                    drawPtr[x] = drawcallbatcher::DrawCall{
                        .modelHandle = mesh[x].handle,
                        .meshId = mesh[x].meshId,
                        .materialHandle = material[x].handle,
                        .transform = (&matrices[startIndex])[x],
                    };
                }
                drawcallbatcher::Batch(drawPtr, iter->count);
            });

        world.system()
            .kind(flecs::PostUpdate)
            .run([](flecs::iter& it) {
                offset.store(0, std::memory_order_relaxed);
            });
    }
}
