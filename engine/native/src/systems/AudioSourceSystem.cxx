#include "playground/systems/AudioSourceSystem.hxx"
#include "playground/components/AudioSourceComponent.hxx"
#include "playground/components/WorldTransformComponent.hxx"
#include <tracy/Tracy.hpp>
#include <audio/Audio.hxx>

namespace playground::ecs::audiosourcesystem {
    void Init(flecs::world world) {
        world.system<AudioSourceComponent, const WorldTransformComponent>("AudioSourceSystem")
            .kind(flecs::PostUpdate)
            .multi_threaded(true)
            .each([](flecs::iter& it, size_t, AudioSourceComponent& audioSource, const WorldTransformComponent& trans) {
                ZoneScopedNC("AudioSourceSystem", tracy::Color::Pink);
                if (audioSource.handle == UINT64_MAX) {
                    audioSource.previousPosition = trans.Position;
                    audioSource.forward = trans.Rotation.Forward();
                    audioSource.handle = audio::CreateAudioSource(
                        audioSource.eventName,
                        trans.Position,
                        trans.Rotation.Up(),
                        trans.Rotation.Forward(),
                        math::Vector3(0.0f, 0.0f, 0.0f)
                    );

                    return;
                }

                // Calculate
                if (
                    audioSource.previousPosition != trans.Position ||
                    audioSource.forward != trans.Rotation.Forward()
                ) {
                    audio::UpdateAudioSource(
                        audioSource.handle,
                        trans.Position,
                        trans.Rotation.Up(),
                        trans.Rotation.Forward(),
                        (audioSource.previousPosition - trans.Position) / it.delta_time()
                    );
                }
            });
    }
}
