#pragma once

#include <math/Vector3.hxx>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace playground::io {
    struct FileHandle;
}

namespace playground::audio {
    struct AudioDevice;
	struct AudioClip;
    
    auto Init(
        std::function < io::FileHandle* (const char*, const char* file)> openFileCallback,
        std::function <std::vector<uint8_t>(io::FileHandle* handle, size_t length)> readFileCallback,
        std::function <void(io::FileHandle* handle, size_t offset)> seekFileCallback,
        std::function <void(io::FileHandle* handle)> closeFileCallback
    ) -> void;
    void SetListenerPosition(
        uint8_t index,
        float x,
        float y,
        float z,
        float vx = 0.0f,
        float vy = 0.0f,
        float vz = 0.0f,
        float fx = 0.0f,
        float fy = 0.0f,
        float fz = 0.0f
    );
    void* LoadBank(std::string_view archiveName, std::string_view name);
    uint32_t CreateAudioSource(
        const char* eventName,
        float x,
        float y,
        float z,
        float vx = 0.0f,
        float vy = 0.0f,
        float vz = 0.0f,
        float fx = 0.0f,
        float fy = 0.0f,
        float fz = 0.0f
    );
    void UpdateAudioSource(
        uint64_t handle,
        math::Vector3 position,
        math::Vector3 velocity,
        math::Vector3 forward
    );
    void SetVolume(float volume);
    auto Shutdown() -> void;
    auto Update() -> void;
}
