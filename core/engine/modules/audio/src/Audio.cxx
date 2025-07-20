#include "audio/Audio.hxx"
#include "audio/AudioSource.hxx"
#include "audio/AudioDevice.hxx"
#include <io/IO.hxx>
#include <fmod_studio.hpp>
#include <fmod.hpp>
#include <fmod_errors.h>
#include <phonon.h>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <shared/JobSystem.hxx>
#include <string>
#include <vector>
#include <iostream>
#include <tracy/Tracy.hpp>

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
        std::vector<std::shared_ptr<jobsystem::JobHandle>> updateAudioSourcesJobs;
        std::vector<std::shared_ptr<AudioSource>> audioSources;
        bool isDirty = false;
        std::mutex audioSourcesMutex;
    };

    struct UserData {
        std::string archiveName;
        std::string fileName;
        uint8_t refCount = 0;
    };

    extern "C" {
        FMOD_DSP_DESCRIPTION* F_CALL FMOD_SteamAudio_Spatialize_GetDSPDescription();
        FMOD_DSP_DESCRIPTION* F_CALL FMOD_SteamAudio_MixerReturn_GetDSPDescription();
        FMOD_DSP_DESCRIPTION* F_CALL FMOD_SteamAudio_Reverb_GetDSPDescription();
        void F_CALL iplFMODInitialize(IPLContext context);
        void F_CALL iplFMODSetSimulationSettings(IPLSimulationSettings simulationSettings);
    }

    FMODInstance* instance = nullptr;

    FMOD_RESULT OpenFile(
        const char* name,
        unsigned int* filesize,
        void** handle,
        void* userdata
    ) {
        UserData* userData = static_cast<UserData*>(userdata);

        try {
            *handle = instance->openFileCallback(userData->archiveName.c_str(), userData->fileName.c_str());
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
        UserData* userData = static_cast<UserData*>(userdata);
     
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
        instance = new FMODInstance();
        FMOD::Studio::System::create(&instance->system);
        instance->system->getCoreSystem(&instance->coreSystem);
        instance->coreSystem->setSoftwareFormat(48000, FMOD_SPEAKERMODE_STEREO, 0);
        instance->coreSystem->setOutput(FMOD_OUTPUTTYPE_AUTODETECT);
        instance->coreSystem->set3DSettings(1.0f, 1.0f, 1.0f);
        instance->openFileCallback = openFileCallback;
        instance->readFileCallback = readFileCallback;
        instance->seekFileCallback = seekFileCallback;
        instance->closeFileCallback = closeFileCallback;

        unsigned int handle = 0;
        instance->coreSystem->registerDSP(FMOD_SteamAudio_Spatialize_GetDSPDescription(), &handle);
        instance->coreSystem->registerDSP(FMOD_SteamAudio_MixerReturn_GetDSPDescription(), &handle);
        instance->coreSystem->registerDSP(FMOD_SteamAudio_Reverb_GetDSPDescription(), &handle);

        IPLContextSettings contextSettings{};
        contextSettings.version = STEAMAUDIO_VERSION;

        IPLContext context = nullptr;
        auto status = iplContextCreate(&contextSettings, &context);

        if (context == nullptr) {
            throw std::runtime_error("Failed to create Steam Audio context");
        }

        iplFMODInitialize(context);
        instance->context = context;

        IPLHRTFSettings hrtfSettings{};
        hrtfSettings.type = IPL_HRTFTYPE_DEFAULT;
        hrtfSettings.volume = 1.0f;

        IPLAudioSettings audioSettings{};
        audioSettings.samplingRate = 44100;
        audioSettings.frameSize = 1024;

        IPLHRTF hrtf = nullptr;
        iplHRTFCreate(context, &audioSettings, &hrtfSettings, &hrtf);

        IPLBinauralEffectSettings effectSettings{};
        effectSettings.hrtf = hrtf;

        IPLBinauralEffect effect = nullptr;
        iplBinauralEffectCreate(context, &audioSettings, &effectSettings, &effect);

        IPLSimulationSettings simSettings = {};
        simSettings.flags = static_cast<IPLSimulationFlags>(IPL_SIMULATIONFLAGS_DIRECT | IPL_SIMULATIONFLAGS_REFLECTIONS);
        simSettings.sceneType = IPL_SCENETYPE_DEFAULT;
        simSettings.reflectionType = IPL_REFLECTIONEFFECTTYPE_PARAMETRIC;
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
        iplSimulatorCreate(context, &simSettings, &simulator);
        instance->simulator = simulator;

        FMOD_RESULT result = instance->system->initialize(512, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL, nullptr);
        if (result != FMOD_OK) {
            throw std::runtime_error("Failed to initialize FMOD: " + std::string(FMOD_ErrorString(result)));
        }

        IPLSceneSettings sceneSettings = {};
        sceneSettings.type = IPL_SCENETYPE_DEFAULT;
        iplSceneCreate(
            instance->context,
            &sceneSettings,
            &instance->scene
        );
        if (instance->scene == nullptr) {
            throw std::runtime_error("Failed to create Steam Audio scene");
        }

        SetListenerPosition(
            0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, -1.0f
        );

        instance->coreSystem->createDSP(
            FMOD_SteamAudio_Spatialize_GetDSPDescription(),
            &instance->spatialDsp
        );

        instance->coreSystem->createDSP(
            FMOD_SteamAudio_MixerReturn_GetDSPDescription(),
            &instance->mixerReturnDsp
        );

        instance->coreSystem->createDSP(
            FMOD_SteamAudio_Reverb_GetDSPDescription(),
            &instance->reverbDsp
        );
    }

    void SetListenerPosition(
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
        FMOD_3D_ATTRIBUTES attributes = {};
        attributes.position = { x, y, z };
        attributes.velocity = { vx, vy, vz };
        attributes.forward = { fx, fy, fz };
        attributes.up = { 0, 1, 0 };

        FMOD_RESULT result = instance->system->setListenerAttributes(0, &attributes);
        if (result != FMOD_OK) {
            printf("FMOD setListenerAttributes error: %s\n", FMOD_ErrorString(result));
        }
    }

    void* LoadBank(std::string_view archiveName, std::string_view name) {
        FMOD_STUDIO_BANK_INFO bankInfo;
        bankInfo.size = sizeof(FMOD_STUDIO_BANK_INFO);
        bankInfo.opencallback = OpenFile;
        bankInfo.closecallback = CloseFile;
        bankInfo.readcallback = ReadFile;
        bankInfo.seekcallback = SeekFile;

        auto userData = UserData{ std::string(archiveName), std::string(name), 0 };

        bankInfo.userdata = &userData;
        bankInfo.userdatalength = sizeof(UserData);

        FMOD::Studio::Bank* bank;

        FMOD_RESULT result = instance->system->loadBankCustom(&bankInfo, FMOD_STUDIO_LOAD_BANK_NORMAL, &bank);

        if (result != FMOD_OK) {
            throw std::runtime_error("Failed to load FMOD bank: " + std::string(FMOD_ErrorString(result)));
        }

        return bank;
    }

    uint32_t CreateAudioSource(
        const char* eventName,
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
        IPLSourceSettings sourceSettings = {};
        sourceSettings.flags = static_cast<IPLSimulationFlags>
            (IPL_DIRECTSIMULATIONFLAGS_DISTANCEATTENUATION |
                IPL_DIRECTSIMULATIONFLAGS_AIRABSORPTION |
                IPL_DIRECTSIMULATIONFLAGS_DIRECTIVITY |
                IPL_DIRECTSIMULATIONFLAGS_OCCLUSION |
                IPL_DIRECTSIMULATIONFLAGS_TRANSMISSION
                );
        IPLSource source = nullptr;
        iplSourceCreate(
            instance->simulator,
            &sourceSettings,
            &source
        );

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
        attributes.position = { x, y, z };
        attributes.velocity = { vx, vy, vz };
        attributes.forward = { fx, fy, fz };
        attributes.up = { 0.0f, 1.0f, 0.0f };

        result = eventInstance->set3DAttributes(&attributes);
        if (result != FMOD_OK) {
            throw std::runtime_error("Failed to set 3d attributes: " + std::string(FMOD_ErrorString(result)));
        }

        instance->isDirty = true;

        std::scoped_lock<std::mutex> lock{ instance->audioSourcesMutex };
        uint64_t handle = instance->audioSources.size();
        instance->audioSources.push_back(
            std::make_shared<AudioSource>(
                source,
                instance->simulator,
                eventInstance,
                x, y, z, vx, vy, vz, fx, fy, fz
            )
        );

        if (instance->audioSources[handle] == nullptr) {
            throw std::runtime_error("Failed to create AudioSource");
        }

        return static_cast<uint32_t>(handle);
    }

    void UpdateAudioSource(
        uint64_t handle,
        math::Vector3 position,
        math::Vector3 velocity,
        math::Vector3 forward
    ) {
        auto setSourceJob = jobsystem::JobHandle::Create("Set Audio Source Data", jobsystem::JobPriority::Low, 50, [handle, position, velocity, forward]() {
            ZoneScopedNC("Set Audio Source Data", tracy::Color::Red);
            instance->audioSources[handle]->SetSourceData(position, velocity, forward);
        });

        std::scoped_lock<std::mutex> lock{ instance->audioSourcesMutex };
        instance->updateAudioSourcesJobs.push_back(setSourceJob);
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
        // If the audio job is running, wait for it to finish
        if (instance->audioJob) {
            while (!instance->audioJob->IsDone()) {}
        }

        auto result = instance->system->update();
        if (result != FMOD_OK) {
            std::cerr << "FMOD update error: " << FMOD_ErrorString(result) << std::endl;
        }
        for (auto& source : instance->audioSources) {
            source->Simulate();
        }
        instance->updateAudioSourcesJobs.clear();

        instance->audioJob = jobsystem::JobHandle::Create("Steam Audio Task", jobsystem::JobPriority::Low, 49, []() {
            iplSimulatorRunDirect(instance->simulator);
        });
        if (instance->updateAudioSourcesJobs.empty()) {
            auto emptyMarkerJob = jobsystem::JobHandle::Create("No Audio Sources", jobsystem::JobPriority::Low, 0, []() {});
            instance->audioJob->AddDependency(emptyMarkerJob);
            jobsystem::Submit(emptyMarkerJob);
        }
        for (auto& job : instance->updateAudioSourcesJobs) {
            instance->audioJob->AddDependency(job);
            jobsystem::Submit(job);
        }
    }

    auto LoadClip(std::vector<float> buffer) -> std::shared_ptr<AudioClip> {
        return nullptr;
    }
}
