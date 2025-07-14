#pragma once

#include <assetloader/RawMaterialData.hxx>
#include <assetloader/RawMeshData.hxx>
#include <assetloader/RawTextureData.hxx>
#include <assetloader/RawCubemapData.hxx>
#include <rendering/Material.hxx>
#include <rendering/Mesh.hxx>
#include <rendering/Shader.hxx>
#include <rendering/Texture.hxx>
#include <audio/AudioClip.hxx>
#include <atomic>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace playground::jobsystem {
    class JobHandle;
}

namespace playground::assetmanager {
    enum ResourceState {
        Created,
        Unloaded,
        Loading,
        Uploaded
    };

    struct ModelHandle {
        uint64_t hash;
        std::atomic<ResourceState> state;
        std::atomic<uint32_t> refCount;
        std::vector<playground::rendering::Mesh> meshes;
        std::vector<assetloader::RawMeshData> rawMeshData;
    };

    struct TextureHandle {
        uint64_t hash;
        std::atomic<ResourceState> state;
        uint32_t refCount;
        std::shared_ptr<assetloader::RawTextureData> data;
        std::shared_ptr<jobsystem::JobHandle> signalUploadCompletionJob;
        uint32_t texture;
    };

    struct MaterialHandle {
        uint64_t hash;
        std::atomic<ResourceState> state;
        std::map<std::string, TextureHandle*> textures;
        uint32_t refCount;
        uint32_t material;
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

    struct CubemapHandle {
        uint64_t hash;
        std::atomic<ResourceState> state;
        uint32_t refCount;
        std::shared_ptr<assetloader::RawCubemapData> data;
        std::vector<std::shared_ptr<assetloader::RawTextureData>> faces;
        std::shared_ptr<jobsystem::JobHandle> signalUploadCompletionJob;
        uint32_t cubemap;
    };

    ModelHandle* LoadModel(const char* name);
    MaterialHandle* LoadMaterial(const char* name);
    ShaderHandle* LoadShader(const char* name);
    TextureHandle* LoadTexture(const char* name, jobsystem::JobHandle* materialUploadJob);
    PhysicsMaterialHandle* LoadPhysicsMaterial(const char* name);
    CubemapHandle* LoadCubemap(const char* name, jobsystem::JobHandle* materialUploadJob);
}
