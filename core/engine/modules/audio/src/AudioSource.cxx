#include "audio/AudioSource.hxx"
#include "audio/AudioClip.hxx"

#include <iostream>

namespace playground::audio {
    AudioSource::AudioSource(float x, float y, float z) {
        SetPosition(x, y, z);
        SetLooping(false);
        SetPitch(1.0);
        SetGain(1.0);
    }

    AudioSource::~AudioSource() {
    }
    
    auto AudioSource::PlayAudioClip(AudioClip* clip) -> void {
        SetClip(clip);
        Start();
        SetLooping(false);
    }

    auto AudioSource::Start() -> void {
    }

    auto AudioSource::Stop() -> void {
    }

    auto AudioSource::Tick() -> void {
    }

    auto AudioSource::SetClip(AudioClip* clip) -> void {
        auto buff = clip->BufferId();
    }

    auto AudioSource::SetPosition(float x, float y, float z) -> void {
    }

    auto AudioSource::SetRotation(float x, float y, float z) -> void {
    }

    auto AudioSource::SetPitch(float value) -> void {
    }

    auto AudioSource::SetGain(float value) -> void {
    }

    auto AudioSource::SetLooping(bool value) -> void {
    }
}
