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
                ZoneScopedNC("AudioSourceSystem", tracy::Color::Pink);
                if (audioSource.handle == UINT64_MAX) {
                    audioSource.previousPosition = trans.position;
                    audioSource.forward = rot.rotation.Forward();
                    audioSource.handle = audio::CreateAudioSource(
                        audioSource.eventName,
                        trans.position,
                        rot.rotation.Up(),
                        rot.rotation.Forward(),
                        math::Vector3(0.0f, 0.0f, 0.0f)
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
                        rot.rotation.Up(),
                        rot.rotation.Forward(),
                        (audioSource.previousPosition - trans.position) / it.delta_time()
                    );
                }
            });
    }
}
