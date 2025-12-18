#include "playground/systems/AudioListenerSystem.hxx"
#include "playground/components/AudioListenerComponent.hxx"
#include "playground/components/WorldTransformComponent.hxx"
#include <audio/Audio.hxx>
#include <tracy/Tracy.hpp>

namespace playground::ecs::audiolistenersystem {
    void Init(flecs::world world) {
        world.system<const AudioListenerComponent, const WorldTransformComponent>("AudioListenerSystem")
            .kind(flecs::PostUpdate)
            .each([](flecs::entity e, const AudioListenerComponent& listener, const WorldTransformComponent& trans) {
                ZoneScopedNC("AudioListenerSystem", tracy::Color::Pink);
                auto forward = trans.Rotation.Forward();
                auto up = trans.Rotation.Up();
                audio::SetListenerPosition(
                    listener.index,
                    trans.Position,
                    up,
                    forward,
                    math::Vector3(0.0f, 0.0f, 0.0f)
                );
            });
    }
} // namespace playground::ecs::audiolistenersystem
