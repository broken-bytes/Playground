#pragma once

#include <assetloader/RawMeshData.hxx>
#include <rendering/Mesh.hxx>
#include <rendering/Shader.hxx>
#include <rendering/Texture.hxx>
#include <audio/AudioClip.hxx>
#include <cstdint>
#include <string>
#include <vector>

namespace playground::assetmanager {
    struct ModelHandle {
        bool isUploaded;
        std::string name;
        uint32_t refCount;
        std::vector<playground::rendering::Mesh> meshes;
        std::vector<assetloader::RawMeshData> rawMeshData;
    };

    struct TextureHandle {
        bool isUploaded;
        std::string name;
        uint32_t refCount;
        playground::rendering::Texture texture;
    };

    struct ShaderHandle {
        bool isUploaded;
        std::string name;
        uint32_t refCount;
        playground::rendering::Shader shader;
    };

    struct AudioHandle {
        std::string name;
        uint32_t refCount;
        playground::audio::AudioClip audio;
    };

    uint32_t LoadModel(const char* name, uint16_t index);
}
