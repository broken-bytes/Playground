#include "playground/systems/CameraSystem.hxx"
#include "playground/DrawCallBatcher.hxx"
#include "playground/components/WorldTransformComponent.hxx"
#include "playground/components/CameraComponent.hxx"
#include <tracy/Tracy.hpp>

namespace playground::ecs::camerasystem {
    std::atomic<uint32_t> offset = 0;

    void Init(flecs::world world) {
        world.system<const WorldTransformComponent, const CameraComponent>("CameraSystem")
            .kind(flecs::PostUpdate)
            .multi_threaded(true)
            .run([](flecs::iter& it) {
                while (it.next()) {
                    ZoneScopedNC("CameraSystem", tracy::Color::Green);
                    auto transform = it.field<const WorldTransformComponent>(0);
                    auto camera = it.field<const CameraComponent>(1);

                    for (int x = 0; x < it.count(); x++) {
                        drawcallbatcher::AddCamera(camera->Order, camera->Fov, camera->NearPlane, camera->FarPlane, transform[x].Position, transform[x].Rotation);
                    }
                }
            });

        world.system("PreRenderSystem")
            .kind(flecs::PostUpdate)
            .run([](flecs::iter& it) {
                offset.store(0, std::memory_order_relaxed);
        });
    }
}
