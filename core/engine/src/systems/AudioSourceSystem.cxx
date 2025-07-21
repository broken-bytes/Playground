#include "playground/systems/AudioSourceSystem.hxx"
#include "playground/components/AudioSourceComponent.hxx"
#include "playground/components/WorldTranslationComponent.hxx"
#include "playground/components/WorldRotationComponent.hxx"
#include <tracy/Tracy.hpp>
#include <audio/Audio.hxx>

namespace playground::ecs::audiosourcesystem {
    void Init(flecs::world world) {
        world.system<AudioSourceComponent, const WorldTranslationComponent, const WorldRotationComponent>("AudioSourceSystem")
            .kind(flecs::PostUpdate)
            .multi_threaded(true)
            .each([](flecs::iter& it, size_t, AudioSourceComponent& audioSource, const WorldTranslationComponent& trans, const WorldRotationComponent& rot) {
                if (audioSource.handle == UINT64_MAX) {
                    audioSource.previousPosition = trans.position;
                    audioSource.forward = rot.rotation.Forward();
                    audioSource.handle = audio::CreateAudioSource(
                        audioSource.eventName,
                        trans.position.X,
                        trans.position.Y,
                        trans.position.Z,
                        0,
                        0,
                        0,
                        audioSource.forward.X,
                        audioSource.forward.Y,
                        audioSource.forward.Z
                    );

                    return;
                }

                // Calculate
                if (
                    audioSource.previousPosition != trans.position ||
                    audioSource.forward != rot.rotation.Forward()
                ) {
                    audio::UpdateAudioSource(
                        audioSource.handle,
                        trans.position,
                        (audioSource.previousPosition - trans.position) / it.delta_time(),
                        rot.rotation.Forward()
                    );
                }
            });
    }
}
