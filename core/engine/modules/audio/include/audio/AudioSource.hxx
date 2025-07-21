#pragma once

#include <math/Vector3.hxx>
#include <fmod_studio.hpp>
#include <fmod.hpp>
#include <fmod_errors.h>
#include <phonon/phonon.h>
#include <cstdint>

namespace playground::audio {
    struct AudioSource {
        FMOD::Studio::EventInstance* fmodEventInstance;
        IPLSource iplSource;
        IPLSimulationInputs settings;
    };
}
