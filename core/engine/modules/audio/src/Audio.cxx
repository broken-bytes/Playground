#include "audio/Audio.hxx"
#include "audio/AudioClip.hxx"
#include "audio/AudioDevice.hxx"
#include <stdexcept>
#include <string>
#include <vector>
#include <SDL3/SDL.h>
#include <phonon.h>

namespace playground::audio {
    IPLContext context = nullptr;
    IPLAudioSettings audioSettings;

    auto Init() -> void {
        if (!SDL_Init(SDL_INIT_AUDIO)) {
			throw std::runtime_error("Failed to initialize SDL audio");
        }

        IPLContextSettings contextSettings = {};
        contextSettings.version = STEAMAUDIO_VERSION;

		iplContextCreate(&contextSettings, &context);
        IPLHRTFSettings hrtfSettings{};
        hrtfSettings.type = IPL_HRTFTYPE_DEFAULT;
        hrtfSettings.volume = 1.0f;

        audioSettings = {};
        audioSettings.samplingRate = 44100;
        audioSettings.frameSize = 1024;

        IPLHRTF hrtf = nullptr;
        iplHRTFCreate(context, &audioSettings, &hrtfSettings, &hrtf);

        IPLBinauralEffectSettings effectSettings{};
        effectSettings.hrtf = hrtf;

        IPLBinauralEffect effect = nullptr;
        iplBinauralEffectCreate(context, &audioSettings, &effectSettings, &effect);
    }

    auto Update() -> void {

    }

    auto LoadClip(std::vector<float> buffer) -> std::shared_ptr<AudioClip> {
        return nullptr;
    }
}
