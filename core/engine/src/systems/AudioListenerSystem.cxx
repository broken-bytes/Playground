#include "playground/systems/AudioListenerSystem.hxx"
#include "playground/components/AudioListenerComponent.hxx"
#include "playground/components/WorldTranslationComponent.hxx"
#include <audio/Audio.hxx>

namespace playground::ecs::audiolistenersystem {
    void Init(flecs::world world) {
        world.system<const AudioListenerComponent, const WorldTranslationComponent>("AudioListenerSystem")
            .kind(flecs::PostUpdate)
            .each([](flecs::entity e, const AudioListenerComponent& listener, const WorldTranslationComponent& trans) {
                audio::SetListenerPosition(
                    listener.index,
                    trans.position.X,
                    trans.position.Y,
                    trans.position.Z,
                    0,
                    0,
                    0,
                    0,
                    0,
                    1
                );
            });
    }
} // namespace playground::ecs::audiolistenersystem
