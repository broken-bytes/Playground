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
#include <shared/Logger.hxx>
#include <math/Math.hxx>
#include <memory>

namespace playground::ecs::rendersystem {
    auto drawCalls = new drawcallbatcher::DrawCall[rendering::MAX_DRAW_CALLS_PER_FRAME];

    std::atomic<uint32_t> offset = 0;

    void Init(flecs::world world) {
        auto translation = ecs::RegisterComponent("WorldTranslationComponent", sizeof(WorldTranslationComponent), alignof(WorldTranslationComponent));
        auto rotation = ecs::RegisterComponent("WorldRotationComponent", sizeof(WorldRotationComponent), alignof(WorldRotationComponent));
        auto scale = ecs::RegisterComponent("WorldScaleComponent", sizeof(WorldScaleComponent), alignof(WorldScaleComponent));
        auto mesh = ecs::RegisterComponent("MeshComponent", sizeof(MeshComponent), alignof(MeshComponent));
        auto material = ecs::RegisterComponent("MaterialComponent", sizeof(MaterialComponent), alignof(MaterialComponent));

        world.system<const WorldTranslationComponent, const WorldRotationComponent, const WorldScaleComponent, const MeshComponent, const MaterialComponent>("RenderSystem")
            .kind(flecs::PostUpdate)
            .multi_threaded(true)
            .run([](flecs::iter& it) {
                while (it.next()) {
                    ZoneScopedNC("RenderSystem", tracy::Color::Green);
                    auto translation = it.field<const WorldTranslationComponent>(0);
                    auto rotation = it.field<const WorldRotationComponent>(1);
                    auto scale = it.field<const WorldScaleComponent>(2);
                    auto mesh = it.field<const MeshComponent>(3);
                    auto material = it.field<const MaterialComponent>(4);
                    auto startIndex = offset.fetch_add(it.count(), std::memory_order_acquire);
                    auto drawPtr = &drawCalls[startIndex];
                    for (int x = 0; x < it.count(); x++) {
                        math::Matrix4x4 mat = math::Mat4FromPRS(
                            translation[x].position,
                            rotation[x].rotation,
                            scale[x].scale
                        );

                        drawPtr[x] = drawcallbatcher::DrawCall{
                            .modelHandle = mesh[x].handle,
                            .meshId = mesh[x].meshId,
                            .materialHandle = material[x].handle,
                            .transform = mat,
                        };
                    }
                    drawcallbatcher::Batch(drawPtr, it.count());
                }
            });

        world.system("PreRenderSystem")
            .kind(flecs::PostUpdate)
            .run([](flecs::iter& it) {
                offset.store(0, std::memory_order_relaxed);
        });
    }
}
/*
world.system<const WorldTranslationComponent, const WorldRotationComponent, const WorldScaleComponent, const MeshComponent, const MaterialComponent>("RenderSystem")
.kind(flecs::PostUpdate)
.multi_threaded(true)
.run([](flecs::iter& it) {
    ZoneScopedNC("RenderSystem", tracy::Color::Green);
    while (it.next()) {
        auto translation = it.field<const WorldTranslationComponent>(0);
        auto rotation = it.field<const WorldRotationComponent>(1);
        auto scale = it.field<const WorldScaleComponent>(2);
        auto mesh = it.field<const MeshComponent>(3);
        auto material = it.field<const MaterialComponent>(4);

        auto startIndex = offset.fetch_add(it.count(), std::memory_order_relaxed);

        auto drawPtr = &drawCalls[startIndex];
        for (int x = 0; x < it.count(); x++) {
            math::Mat4FromPRS(
                reinterpret_cast<math::Vector3*>(&translation),
                reinterpret_cast<math::Quaternion*>(&rotation),
                reinterpret_cast<math::Vector3*>(&scale),
                &matrices[startIndex + x]
            );
            drawPtr[x] = drawcallbatcher::DrawCall{
                .modelHandle = mesh[x].handle,
                .meshId = mesh[x].meshId,
                .materialHandle = material[x].handle,
                .transform = (&matrices[startIndex])[x],
            };
        }
        drawcallbatcher::Batch(drawPtr, it.count());
    }
    });
*/
