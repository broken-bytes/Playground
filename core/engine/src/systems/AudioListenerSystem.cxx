#include "playground/systems/AudioListenerSystem.hxx"
#include "playground/components/AudioListenerComponent.hxx"
#include "playground/components/WorldTranslationComponent.hxx"
#include "playground/components/WorldRotationComponent.hxx"
#include <audio/Audio.hxx>

namespace playground::ecs::audiolistenersystem {
    void Init(flecs::world world) {
        world.system<const AudioListenerComponent, const WorldTranslationComponent, const WorldRotationComponent>("AudioListenerSystem")
            .kind(flecs::PostUpdate)
            .each([](flecs::entity e, const AudioListenerComponent& listener, const WorldTranslationComponent& trans, const WorldRotationComponent& rot) {
                auto forward = rot.rotation.Forward();
                auto up = rot.rotation.Up();
                audio::SetListenerPosition(
                    listener.index,
                    trans.position,
                    up,
                    forward,
                    math::Vector3(0.0f, 0.0f, 0.0f)
                );
            });
    }
} // namespace playground::ecs::audiolistenersystem
