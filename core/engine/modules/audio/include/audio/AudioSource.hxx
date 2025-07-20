#pragma once

#include <math/Vector3.hxx>
#include <fmod_studio.hpp>
#include <fmod.hpp>
#include <fmod_errors.h>
#include <phonon.h>
#include <cstdint>

namespace playground::audio {
    class AudioSource {
    public:
        AudioSource(
            IPLSource iplSource,
            IPLSimulator simulator,
            FMOD::Studio::EventInstance* fmodEventInstance,
            float x = 0,
            float y = 0,
            float z = 0,
            float vx = 0.0f,
            float vy = 0.0f,
            float vz = 0.0f,
            float fx = 0.0f,
            float fy = 0.0f,
            float fz = 0.0f
        );
        ~AudioSource();

        void Simulate();
        void SetSourceData(
            math::Vector3 position,
            math::Vector3 velocity,
            math::Vector3 forward
        );
    private:
        IPLSource _iplSource;
        FMOD::Studio::EventInstance* _fmodEventInstance;
        FMOD::DSP* _spatialDsp;
    };
}
