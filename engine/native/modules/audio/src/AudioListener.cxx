#include "audio/AudioListener.hxx"

namespace playground::audio {
    AudioListener::AudioListener() {
        SetPosition(0, 0, 1.0f);
        SetRotation(0, 0, 0);
    }

    AudioListener::~AudioListener() {

    }

    auto AudioListener::SetPosition(float x, float y, float z) -> void {
    }

    auto AudioListener::SetRotation(float x, float y, float z) -> void {
    }
}
