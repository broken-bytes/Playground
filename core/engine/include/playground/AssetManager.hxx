#pragma once

#include <assetloader/RawMaterialData.hxx>
#include <assetloader/RawMeshData.hxx>
#include <rendering/Material.hxx>
#include <rendering/Mesh.hxx>
#include <rendering/Shader.hxx>
#include <rendering/Texture.hxx>
#include <audio/AudioClip.hxx>
#include <atomic>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace playground::assetmanager {
    enum ResourceState {
        Created,
        Unloaded,
        Loading,
        Uploaded
    };

    struct ModelHandle {
        uint64_t hash;
        ResourceState state;
        std::atomic<uint32_t> refCount;
        std::vector<playground::rendering::Mesh> meshes;
        std::vector<assetloader::RawMeshData> rawMeshData;
    };

    struct MaterialHandle {
        uint64_t hash;
        ResourceState state;
        uint32_t refCount;
        uint32_t material;
    };

    struct TextureHandle {
        ResourceState state;
        std::string name;
        uint32_t refCount;
        playground::rendering::Texture texture;
    };

    struct ShaderHandle {
        uint64_t hash;
        ResourceState state;
        uint32_t refCount;
        std::string bytecode;
    };

    struct AudioHandle {
        ResourceState state;
        uint32_t refCount;
        playground::audio::AudioClip audio;
    };

    ModelHandle* LoadModel(std::string_view name);
    MaterialHandle* LoadMaterial(std::string_view name);
    ShaderHandle* LoadShader(std::string_view name);
}
