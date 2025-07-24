#include "audio/Audio.hxx"
#include "audio/AudioSource.hxx"
#include "audio/AudioDevice.hxx"
#include <io/IO.hxx>
#include <fmod_studio.hpp>
#include <fmod.hpp>
#include <fmod_errors.h>
#include <phonon/phonon.h>
#include <steamaudio_fmod.h>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <shared/Job.hxx>
#include <shared/JobHandle.hxx>
#include <shared/JobSystem.hxx>
#include <shared/Logger.hxx>
#include <string>
#include <vector>
#include <iostream>
#include <tracy/Tracy.hpp>
#ifdef _WIN32
#include <Windows.h>
#endif

namespace playground::audio {
    struct FMODInstance
    {
        FMOD::System* coreSystem;
        FMOD::Studio::System* system;
        const char* dialogueString;
        std::function<io::FileHandle* (const char* archive, const char* file)> openFileCallback;
        std::function<std::vector<uint8_t>(io::FileHandle* handle, size_t length)> readFileCallback;
        std::function<void(io::FileHandle* handle, size_t offset)> seekFileCallback;
        std::function<void(io::FileHandle* handle)> closeFileCallback;
        IPLSimulator simulator;
        IPLContext context;
        IPLScene scene;
        FMOD::DSP* spatialDsp;
        FMOD::DSP* mixerReturnDsp;
        FMOD::DSP* reverbDsp;
        std::shared_ptr<jobsystem::JobHandle> audioJob;
        std::vector<AudioSource> audioSources;
        bool isDirty = false;
        std::mutex audioSourcesMutex;
    };

    struct UserData {
        char archiveName[260];
        char fileName[260];
        uint8_t refCount = 0;
    };

    FMODInstance* instance = nullptr;

    void SetSourceOutput(AudioSource& source);

    FMOD_RESULT OpenFile(
        const char* name,
        unsigned int* filesize,
        void** handle,
        void* userdata
    ) {
        UserData* userData = static_cast<UserData*>(userdata);

        try {
            *handle = instance->openFileCallback(userData->archiveName, userData->fileName);
            *filesize = static_cast<io::FileHandle*>(*handle)->fileSize;

            if (*handle == nullptr) {
                return FMOD_ERR_FILE_NOTFOUND;
            }

            return FMOD_OK;
        }
        catch (std::runtime_error) {
            return FMOD_ERR_FILE_NOTFOUND;
        }
    }

    FMOD_RESULT ReadFile(
        void* handle,
        void* buffer,
        unsigned int sizebytes,
        unsigned int* bytesread,
        void* userdata
    ) {
        try {
            auto data = instance->readFileCallback(static_cast<io::FileHandle*>(handle), sizebytes);
            *bytesread = static_cast<unsigned int>(data.size());
            std::memcpy(buffer, data.data(), *bytesread);

            return FMOD_OK;
        }
        catch (std::runtime_error err) {
            return FMOD_ERR_FILE_BAD;
        }
    }

    FMOD_RESULT SeekFile(
        void* handle,
        unsigned int pos,
        void* userdata
    ) {
        try {
            instance->seekFileCallback(static_cast<io::FileHandle*>(handle), pos);
            return FMOD_OK;
        }
        catch (std::runtime_error err) {
            return FMOD_ERR_FILE_BAD;
        }
    }

    FMOD_RESULT CloseFile(
        void* handle,
        void* userdata
    ) {     
        try {
            instance->closeFileCallback(static_cast<io::FileHandle*>(handle));
            return FMOD_OK;
        }
        catch (std::runtime_error) {
            return FMOD_ERR_FILE_BAD;
        }
    }

    auto Init(
        std::function<io::FileHandle* (const char* archive, const char* file)> openFileCallback,
        std::function<std::vector<uint8_t>(io::FileHandle* handle, size_t length)> readFileCallback,
        std::function<void(io::FileHandle* handle, size_t offset)> seekFileCallback,
        std::function<void(io::FileHandle* handle)> closeFileCallback
    ) -> void {
        logging::logger::SetupSubsystem("audio");
        logging::logger::Info("Initializing FMOD Audio System...", "audio");
        FMOD::Debug_Initialize(FMOD_DEBUG_LEVEL_LOG, FMOD_DEBUG_MODE_TTY, 0, 0);
        void* extraDriverData = NULL;
        CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

        instance = new FMODInstance();
        FMOD::Studio::System::create(&instance->system);
        instance->system->getCoreSystem(&instance->coreSystem);
        instance->coreSystem->setSoftwareFormat(0, FMOD_SPEAKERMODE_DEFAULT, 0);

        FMOD_RESULT result = instance->system->initialize(1024, FMOD_STUDIO_INIT_NORMAL | FMOD_STUDIO_INIT_LIVEUPDATE, FMOD_INIT_NORMAL, extraDriverData);
        if (result != FMOD_OK) {
            throw std::runtime_error("Failed to initialize FMOD: " + std::string(FMOD_ErrorString(result)));
        }

        instance->system->setNumListeners(1);
        instance->system->setListenerWeight(0, 1);

        instance->openFileCallback = openFileCallback;
        instance->readFileCallback = readFileCallback;
        instance->seekFileCallback = seekFileCallback;
        instance->closeFileCallback = closeFileCallback;

        IPLContextSettings contextSettings{};
        contextSettings.version = STEAMAUDIO_VERSION;
        contextSettings.flags = IPL_CONTEXTFLAGS_VALIDATION;
        contextSettings.simdLevel = IPL_SIMDLEVEL_AVX;

        IPLContext context = nullptr;
        auto steamAudioResult = iplContextCreate(&contextSettings, &context);
        if (steamAudioResult != IPL_STATUS_SUCCESS || context == nullptr) {
            throw std::runtime_error("Failed to create Steam Audio context: " + std::to_string(result));
        }

        iplFMODInitialize(context);
        instance->context = context;

        unsigned int handle = 0;
        instance->coreSystem->registerDSP(FMOD_SteamAudio_Spatialize_GetDSPDescription(), &handle);
        instance->coreSystem->registerDSP(FMOD_SteamAudio_MixerReturn_GetDSPDescription(), &handle);
        instance->coreSystem->registerDSP(FMOD_SteamAudio_Reverb_GetDSPDescription(), &handle);

        IPLHRTFSettings hrtfSettings{};
        hrtfSettings.type = IPL_HRTFTYPE_DEFAULT;
        hrtfSettings.volume = 1.0f;

        IPLAudioSettings audioSettings{};
        audioSettings.samplingRate = 48000;
        audioSettings.frameSize = 1024;

        IPLHRTF hrtf = nullptr;
        steamAudioResult = iplHRTFCreate(context, &audioSettings, &hrtfSettings, &hrtf);
        if (steamAudioResult != IPL_STATUS_SUCCESS || hrtf == nullptr) {
            throw std::runtime_error("Failed to create Steam Audio HRTF: " + std::to_string(result));
        }
        iplFMODSetHRTF(hrtf);

        IPLBinauralEffectSettings effectSettings{};
        effectSettings.hrtf = hrtf;

        IPLBinauralEffect effect = nullptr;
        steamAudioResult = iplBinauralEffectCreate(context, &audioSettings, &effectSettings, &effect);
        if (steamAudioResult != IPL_STATUS_SUCCESS || effect == nullptr) {
            throw std::runtime_error("Failed to create Steam Audio binaural effect: " + std::to_string(steamAudioResult));
        }

        IPLSimulationSettings simSettings = {};
        simSettings.flags = static_cast<IPLSimulationFlags>(IPL_SIMULATIONFLAGS_DIRECT | IPL_SIMULATIONFLAGS_REFLECTIONS | IPL_SIMULATIONFLAGS_PATHING);
        simSettings.sceneType = IPL_SCENETYPE_DEFAULT;
        simSettings.reflectionType = IPL_REFLECTIONEFFECTTYPE_HYBRID;
        simSettings.maxNumOcclusionSamples = 8;
        simSettings.maxNumRays = 128;
        simSettings.numDiffuseSamples = 8;
        simSettings.maxDuration = .5f;
        simSettings.maxOrder = 1;
        simSettings.maxNumSources = 16;
        simSettings.numThreads = 2;
        simSettings.rayBatchSize = 0;
        simSettings.numVisSamples = 8;
        simSettings.samplingRate = 48000;
        simSettings.frameSize = 1024;
        simSettings.openCLDevice = nullptr;
        simSettings.radeonRaysDevice = nullptr;
        simSettings.tanDevice = nullptr;

        iplFMODSetSimulationSettings(simSettings);

        IPLSimulator simulator = nullptr;
        steamAudioResult = iplSimulatorCreate(context, &simSettings, &simulator);
        if (steamAudioResult != IPL_STATUS_SUCCESS || simulator == nullptr) {
            throw std::runtime_error("Failed to create Steam Audio simulator: " + std::to_string(steamAudioResult));
        }
        instance->simulator = simulator;

        IPLSceneSettings sceneSettings = {};
        sceneSettings.type = IPL_SCENETYPE_DEFAULT;
        steamAudioResult = iplSceneCreate(
            instance->context,
            &sceneSettings,
            &instance->scene
        );
        if (steamAudioResult != IPL_STATUS_SUCCESS || instance->scene == nullptr) {
            throw std::runtime_error("Failed to create Steam Audio scene: " + std::to_string(steamAudioResult));
        }

        iplSimulatorSetScene(instance->simulator, instance->scene);

        auto fillerJob = jobsystem::Job{
            .Name = "Audio Filler Job",
            .Priority = jobsystem::JobPriority::Low,
            .Color = tracy::Color::DarkSeaGreen1,
            .Dependencies = {},
            .Task = []() {
                // This job is just a filler to not have the audio system start with no job to wait for.
                std::this_thread::yield();
            }
        };

        instance->audioJob = jobsystem::Submit(fillerJob);
    }

    void SetListenerPosition(
        uint8_t index,
        math::Vector3 position,
        math::Vector3 up,
        math::Vector3 forward,
        math::Vector3 velocity
    ) {
        FMOD_3D_ATTRIBUTES attributes = {};
        attributes.position = FMOD_VECTOR{ position.X, position.Y, position.Z };
        attributes.velocity = FMOD_VECTOR{ velocity.X, velocity.Y, velocity.Z };
        attributes.forward = FMOD_VECTOR{ forward.X, forward.Y, forward.Z };
        attributes.up = FMOD_VECTOR{ up.X, up.Y, up.Z };

        FMOD_RESULT result = instance->system->setListenerAttributes(index, &attributes);
        if (result != FMOD_OK) {
            logging::logger::Error("FMOD setListenerAttributes error: " + std::string(FMOD_ErrorString(result)), "audio");
        }
    }

    void* LoadBank(std::string_view archiveName, std::string_view name) {
        FMOD_STUDIO_BANK_INFO bankInfo;
        bankInfo.size = sizeof(FMOD_STUDIO_BANK_INFO);
        bankInfo.opencallback = OpenFile;
        bankInfo.closecallback = CloseFile;
        bankInfo.readcallback = ReadFile;
        bankInfo.seekcallback = SeekFile;

        auto* userData = new UserData();
        strcpy(userData->archiveName, std::string(archiveName).c_str());
        strcpy(userData->fileName, std::string(name).c_str());
        userData->refCount = 0;
        bankInfo.userdata = userData;
        bankInfo.userdatalength = sizeof(UserData);

        FMOD::Studio::Bank* bank;

        FMOD_RESULT result = instance->system->loadBankCustom(&bankInfo, FMOD_STUDIO_LOAD_BANK_NORMAL, &bank);

        delete userData;

        if (result != FMOD_OK) {
            throw std::runtime_error("Failed to load FMOD bank: " + std::string(FMOD_ErrorString(result)));
        }

        return bank;
    }

    uint32_t CreateAudioSource(
        const char* eventName,
        math::Vector3 position,
        math::Vector3 up,
        math::Vector3 forward,
        math::Vector3 velocity
    ) {
        IPLSourceSettings sourceSettings = {};
        sourceSettings.flags = static_cast<IPLSimulationFlags>(IPL_SIMULATIONFLAGS_DIRECT | IPL_SIMULATIONFLAGS_REFLECTIONS | IPL_SIMULATIONFLAGS_PATHING);
        IPLSource source = nullptr;
        auto steamAudioResult = iplSourceCreate(
            instance->simulator,
            &sourceSettings,
            &source
        );
        if (steamAudioResult != IPL_STATUS_SUCCESS || source == nullptr) {
            throw std::runtime_error("Failed to create Steam Audio source: " + std::to_string(steamAudioResult));
        }

        iplSourceAdd(
            source,
            instance->simulator
        );
        iplSimulatorCommit(instance->simulator);

        auto iplHandle = iplFMODAddSource(source);

        FMOD::Studio::EventDescription* description;
        auto result = instance->system->getEvent(eventName, &description);
        if (result != FMOD_OK) {
            throw std::runtime_error("Failed to get FMOD event description: " + std::string(FMOD_ErrorString(result)));
        }

        FMOD::Studio::EventInstance* eventInstance;
        result = description->createInstance(&eventInstance);
        if (result != FMOD_OK) {
            throw std::runtime_error("Failed to create FMOD event instance: " + std::string(FMOD_ErrorString(result)));
        }

        FMOD_3D_ATTRIBUTES attributes = {};
        attributes.position = FMOD_VECTOR{ position.X, position.Y, position.Z };
        attributes.velocity = FMOD_VECTOR{ velocity.X, velocity.Y, velocity.Z };
        attributes.forward = FMOD_VECTOR{ forward.X, forward.Y, forward.Z };
        attributes.up = FMOD_VECTOR{ up.X, up.Y, up.Z };

        result = eventInstance->set3DAttributes(&attributes);
        if (result != FMOD_OK) {
            throw std::runtime_error("Failed to set 3d attributes: " + std::string(FMOD_ErrorString(result)));
        }

        instance->isDirty = true;

        std::scoped_lock<std::mutex> lock{ instance->audioSourcesMutex };
        uint64_t handle = instance->audioSources.size();;

        eventInstance->setVolume(1.0f);

        eventInstance->setCallback(
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

        eventInstance->start();

        AudioSource audioSource = {
            .fmodEventInstance = eventInstance,
            .iplSource = source,
            .handle = iplHandle
        };

        instance->audioSources.push_back(audioSource);

        return static_cast<uint32_t>(handle);
    }

    void UpdateAudioSource(
        uint64_t handle,
        math::Vector3 position,
        math::Vector3 up,
        math::Vector3 forward,
        math::Vector3 velocity
    ) {
        FMOD_3D_ATTRIBUTES attributes = {};
        attributes.position = FMOD_VECTOR{ position.X, position.Y, position.Z };
        attributes.velocity = FMOD_VECTOR{ velocity.X, velocity.Y, velocity.Z };
        attributes.forward = FMOD_VECTOR{ forward.X, forward.Y, forward.Z };
        attributes.up = FMOD_VECTOR{ up.X, up.Y, up.Z };

        FMOD_RESULT result = instance->audioSources[handle].fmodEventInstance->set3DAttributes(&attributes);
        if (result != FMOD_OK) {
            std::cerr << "FMOD set3DAttributes error: " << FMOD_ErrorString(result) << std::endl;
        }
    }

    void SetVolume(float volume) {
        ZoneScopedNC("Set Volume", tracy::Color::Green);
        FMOD::Studio::Bus* masterBus = nullptr;
        auto result = instance->system->getBus("bus:/", &masterBus);
        if (result != FMOD_OK) {
            std::cerr << "Failed to get master bus: " << FMOD_ErrorString(result) << std::endl;
            return;
        }
        if (masterBus) {
            masterBus->setVolume(volume);
        } else {
            std::cerr << "Failed to get master bus" << std::endl;
        }
    }

    auto Shutdown() -> void {
        instance->system->release();
        delete instance;
    }

    auto Update() -> void {
        ZoneScopedNC("Audio Update", tracy::Color::Blue);

        if (instance->isDirty) {
            iplSceneCommit(instance->scene);
            instance->isDirty = false;
        }

        {
            instance->audioJob->Wait();

            auto directJob = jobsystem::Job{
                .Name = "AUDIO_DIRECT_JOB",
                .Priority = jobsystem::JobPriority::High,
                .Color = tracy::Color::Purple4,
                .Dependencies = {},
                .Task = []() {
                        ZoneScopedNC("Audio Direct Job", tracy::Color::Purple4);
                    for (auto& source : instance->audioSources) {
                        ZoneScopedNC("Set Source Inputs", tracy::Color::Purple1);
                    }
                    iplSimulatorRunDirect(instance->simulator);
                }
            };

            auto reflectionsJob = jobsystem::Job{
                .Name = "AUDIO_REFLECTIONS_JOB",
                .Priority = jobsystem::JobPriority::High,
                .Color = tracy::Color::Purple4,
                .Dependencies = {},
                .Task = []() {
                    ZoneScopedNC("Audio Reflections Job", tracy::Color::Purple4);
                    for (auto& source : instance->audioSources) {
                        ZoneScopedNC("Set Source Inputs", tracy::Color::Purple1);
                    }
                    iplSimulatorRunReflections(instance->simulator);
                }
            };

            auto pathingJob = jobsystem::Job{
                .Name = "AUDIO_PATHING_JOB",
                .Priority = jobsystem::JobPriority::High,
                .Color = tracy::Color::Purple4,
                .Dependencies = {},
                .Task = []() {
                    ZoneScopedNC("Audio Pathing Job", tracy::Color::Purple4);
                    for (auto& source : instance->audioSources) {
                        ZoneScopedNC("Set Source Inputs", tracy::Color::Purple1);
                    }
                    iplSimulatorRunPathing(instance->simulator);
                }
            };

            auto completionJob = jobsystem::Job{
                .Name = "AUDIO_COMPLETION_JOB",
                .Priority = jobsystem::JobPriority::High,
                .Color = tracy::Color::Purple4,
                .Dependencies = { directJob, reflectionsJob, pathingJob },
                .Task = []() {
                    ZoneScopedNC("Audio Completion Job", tracy::Color::Purple4);
                    for (auto& source : instance->audioSources) {
                        ZoneScopedNC("Set Source Outputs", tracy::Color::Purple1);
                        SetSourceOutput(source);
                    }
                    auto result = instance->system->update();
                    if (result != FMOD_OK) {
                        std::cerr << "FMOD update error: " << FMOD_ErrorString(result) << std::endl;
                    }
                }
            };

            instance->audioJob = jobsystem::Submit(completionJob);
        }
    }

    void SetSourceOutput(AudioSource& source) {
        if (source.didSetupDSP) {
            return; // Already set up
        }

        FMOD::ChannelGroup* channelGroup = nullptr;
        auto result = source.fmodEventInstance->getChannelGroup(&channelGroup);
        if (result != FMOD_OK) {
            return;
        }

        if (channelGroup) {
            int dspCount = 0;
            channelGroup->getNumDSPs(&dspCount);

            float volume = 1.0f;
            channelGroup->getVolume(&volume);

            FMOD_STUDIO_PLAYBACK_STATE playbackState;
            source.fmodEventInstance->getPlaybackState(&playbackState);

            for (int i = 0; i < dspCount; ++i) {
                FMOD::DSP* dsp = nullptr;
                channelGroup->getDSP(i, &dsp);

                char name[256] = {};
                dsp->getInfo(name, nullptr, nullptr, nullptr, nullptr);

                if (strstr(name, "Steam Audio Spatializer")) {
                    ZoneScopedNC("Set Source Output", tracy::Color::Purple1);
                    dsp->setParameterInt(IPL_SPATIALIZE_APPLY_OCCLUSION, 1); // Occlusion enabled
                    dsp->setParameterInt(IPL_SPATIALIZE_APPLY_TRANSMISSION, 1); // Transmission enabled
                    dsp->setParameterInt(IPL_SPATIALIZE_TRANSMISSION_TYPE, 1); // Transmission type set to 3-band EQ
                    dsp->setParameterInt(IPL_SPATIALIZE_APPLY_DISTANCEATTENUATION, 1); // Distance attenuation enabled
                    dsp->setParameterInt(IPL_SPATIALIZE_APPLY_AIRABSORPTION, 1); // Air absorption enabled
                    dsp->setParameterInt(IPL_SPATIALIZE_APPLY_DIRECTIVITY, 1); // Directivity enabled
                    dsp->setParameterFloat(IPL_SPATIALIZE_DIRECT_MIXLEVEL, 1);
                    dsp->setParameterBool(IPL_SPATIALIZE_REFLECTIONS_BINAURAL, true); // Binaural reflections enabled
                    dsp->setParameterInt(IPL_SPATIALIZE_SIMULATION_OUTPUTS_HANDLE, source.handle);
                    dsp->setParameterInt(IPL_SPATIALIZE_APPLY_PATHING, 1); // Pathing enabled
                    // Commit the changes to the DSP
                    dsp->setActive(true);
                    source.didSetupDSP = true;
                }
            }
        }
    }
}
