#include "audio/AudioSource.hxx"
#include "audio/AudioClip.hxx"
#include <fmod_studio.hpp>
#include <fmod.hpp>
#include <fmod_errors.h>
#include <phonon.h>
#include <stdexcept>
#include <iostream>

namespace playground::audio {
    AudioSource::AudioSource(
        IPLSource iplSource,
        IPLSimulator simulator,
        FMOD::Studio::EventInstance* fmodEventInstance,
        float x,
        float y,
        float z,
        float vx,
        float vy,
        float vz,
        float fx,
        float fy,
        float fz
    ) {
        this->_iplSource = iplSource;
        this->_fmodEventInstance = fmodEventInstance;

        if (this->_iplSource == nullptr || this->_fmodEventInstance == nullptr) {
            throw std::runtime_error("AudioSource: iplSource or fmodEventInstance is null");
        }

        iplSourceAdd(
            iplSource,
            simulator
        );

        auto result = _fmodEventInstance->start();
        if (result != FMOD_OK) {
            throw std::runtime_error("Failed to start FMOD event instance: " + std::string(FMOD_ErrorString(result)));
        }
        if (_fmodEventInstance->isValid() == false) {
            throw std::runtime_error("FMOD event instance is not valid");
        }

        _fmodEventInstance->setCallback(
            [](FMOD_STUDIO_EVENT_CALLBACK_TYPE type, FMOD_STUDIO_EVENTINSTANCE* event, void* parameters) -> FMOD_RESULT {
                switch (type) {
                case FMOD_STUDIO_EVENT_CALLBACK_CREATED:
                    return FMOD_OK;
                case FMOD_STUDIO_EVENT_CALLBACK_DESTROYED:
                    return FMOD_OK;
                case FMOD_STUDIO_EVENT_CALLBACK_STARTING:
                    return FMOD_OK;
                case FMOD_STUDIO_EVENT_CALLBACK_STARTED:
                    return FMOD_OK;
                case FMOD_STUDIO_EVENT_CALLBACK_RESTARTED:
                    return FMOD_OK;
                case FMOD_STUDIO_EVENT_CALLBACK_STOPPED:
                    return FMOD_OK;
                case FMOD_STUDIO_EVENT_CALLBACK_START_FAILED:
                    return FMOD_OK;
                default:
                    return FMOD_OK;
                }
                return FMOD_OK;
            }
        );
    }

    AudioSource::~AudioSource() {
        if (_iplSource) {
            iplSourceRelease(&_iplSource);
            _iplSource = nullptr;
        }
        if (_fmodEventInstance) {
            _fmodEventInstance->stop(FMOD_STUDIO_STOP_ALLOWFADEOUT);
            _fmodEventInstance->release();
            _fmodEventInstance = nullptr;
        }
    }

    void AudioSource::Simulate() {
        IPLSimulationOutputs outputs = {};
        iplSourceGetOutputs(
            _iplSource,
            static_cast<IPLSimulationFlags>
            (IPL_DIRECTSIMULATIONFLAGS_DISTANCEATTENUATION |
                IPL_DIRECTSIMULATIONFLAGS_AIRABSORPTION |
                IPL_DIRECTSIMULATIONFLAGS_DIRECTIVITY |
                IPL_DIRECTSIMULATIONFLAGS_OCCLUSION |
                IPL_DIRECTSIMULATIONFLAGS_TRANSMISSION
                ),
            &outputs
        );

        FMOD::ChannelGroup* channelGroup = nullptr;
        auto result = _fmodEventInstance->getChannelGroup(&channelGroup);
        if (result != FMOD_OK) {
            return;
        }

        if (channelGroup) {
            int dspCount = 0;
            channelGroup->getNumDSPs(&dspCount);

            for (int i = 0; i < dspCount; ++i) {
                FMOD::DSP* dsp = nullptr;
                channelGroup->getDSP(i, &dsp);

                char name[256] = {};
                dsp->getInfo(name, nullptr, nullptr, nullptr, nullptr);

                if (strstr(name, "Steam Audio Spatializer")) {
                    dsp->setParameterInt(0, 1);
                    dsp->setParameterData(2, &outputs, sizeof(outputs));
                }
            }
        }
    }

    void AudioSource::SetSourceData(
        math::Vector3 position,
        math::Vector3 velocity,
        math::Vector3 forward
    ) {
        FMOD_3D_ATTRIBUTES attributes = {};
        attributes.position = reinterpret_cast<const FMOD_VECTOR&>(position);
        attributes.velocity = reinterpret_cast<const FMOD_VECTOR&>(velocity);
        attributes.forward = reinterpret_cast<const FMOD_VECTOR&>(forward);
        attributes.up = { 0, 1, 0 };

        FMOD_RESULT result = _fmodEventInstance->set3DAttributes(&attributes);

        if (result != FMOD_OK) {
            printf("FMOD set3DAttributes error: %s\n", FMOD_ErrorString(result));
        }

        IPLVector3 iplPosition = reinterpret_cast<const IPLVector3&>(position);
        IPLVector3 iplForward = reinterpret_cast<const IPLVector3&>(forward);

        IPLSimulationInputs inputs = {};
        inputs.source.origin = iplPosition;
        inputs.source.ahead = iplForward;
        inputs.source.up = { 0.0f, 1.0f, 0.0f };

        iplSourceSetInputs(
            _iplSource,
            static_cast<IPLSimulationFlags>
            (IPL_DIRECTSIMULATIONFLAGS_DISTANCEATTENUATION |
                IPL_DIRECTSIMULATIONFLAGS_AIRABSORPTION |
                IPL_DIRECTSIMULATIONFLAGS_DIRECTIVITY |
                IPL_DIRECTSIMULATIONFLAGS_OCCLUSION |
                IPL_DIRECTSIMULATIONFLAGS_TRANSMISSION
                ),
            &inputs);
    }
}
