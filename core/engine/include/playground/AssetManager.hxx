#pragma once

#include <assetloader/RawMaterialData.hxx>
#include <assetloader/RawMeshData.hxx>
#include <assetloader/RawTextureData.hxx>
#include <rendering/Material.hxx>
#include <rendering/Mesh.hxx>
#include <rendering/Shader.hxx>
#include <rendering/Texture.hxx>
#include <audio/AudioClip.hxx>
#include <atomic>
#include <cstdint>
#include <memory>
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
        uint64_t hash;
        ResourceState state;
        uint32_t refCount;
        std::shared_ptr<assetloader::RawTextureData> data;
        uint32_t texture;
    };

    struct ShaderHandle {
        uint64_t hash;
        ResourceState state;
        uint32_t refCount;
        std::string vertexShader;
        std::string pixelShader;
    };

    struct AudioHandle {
        ResourceState state;
        uint32_t refCount;
        playground::audio::AudioClip audio;
    };

    struct PhysicsMaterialHandle {
        uint64_t hash;
        ResourceState state;
        uint32_t refCount;
        uint32_t material;
    };

    ModelHandle* LoadModel(const char* name);
    MaterialHandle* LoadMaterial(const char* name);
    ShaderHandle* LoadShader(const char* name);
    TextureHandle* LoadTexture(const char* name);
    PhysicsMaterialHandle* LoadPhysicsMaterial(const char* name);
}
