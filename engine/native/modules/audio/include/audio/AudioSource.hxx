#pragma once

#include <cstdint>

namespace FMOD::Studio
{
    class EventInstance;
}

namespace playground::audio {
    struct AudioSource {
        FMOD::Studio::EventInstance* fmodEventInstance;
        bool didSetupDSP = false;
    };
}
