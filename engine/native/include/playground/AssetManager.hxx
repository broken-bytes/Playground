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
#include <shared/Job.hxx>
#include <shared/JobHandle.hxx>
#include <shared/JobSystem.hxx>
#include <atomic>
#include <cstdint>
#include <functional>
#include <optional>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <vector>


namespace playground::io {
    struct FileHandle;
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
        assetloader::RawTextureData* data;
        uint32_t texture;
        std::shared_ptr<jobsystem::JobHandle> uploadJob;
        std::shared_ptr<jobsystem::JobHandle> completionMarkerJob;
    };

    struct CubemapHandle {
        uint64_t hash;
        std::atomic<ResourceState> state;
        uint32_t refCount;
        std::shared_ptr<assetloader::RawCubemapData> data;
        std::vector<std::shared_ptr<assetloader::RawTextureData>> faces;
        uint32_t cubemap;
        std::shared_ptr<jobsystem::JobHandle> uploadJob;
        std::shared_ptr<jobsystem::JobHandle> completionMarkerJob;
    };

    struct MaterialHandle {
        uint64_t hash;
        std::atomic<ResourceState> state;
        std::map<std::string, TextureHandle*> textures;
        std::map<std::string, CubemapHandle*> cubemaps;
        std::map<std::string, float> floats;
        uint32_t refCount;
        uint32_t material;
        void (*onCompletion)(uint32_t);
    };

    struct ShaderHandle {
        uint64_t hash;
        ResourceState state;
        uint32_t refCount;
        std::string vertexShader;
        std::string pixelShader;
    };

    struct AudioHandle {
        uint64_t hash;
        ResourceState state;
        uint32_t refCount;
        void* audioBank;
    };

    struct PhysicsMaterialHandle {
        uint64_t hash;
        ResourceState state;
        uint32_t refCount;
        uint32_t material;
    };

    ModelHandle* LoadModel_C(const char* name);
    ModelHandle* LoadModel(std::string name);
    MaterialHandle* LoadMaterial_C(const char* name, void (*onCompletion)(uint32_t) = nullptr);
    MaterialHandle* LoadMaterial(std::string name, void (*onCompletion)(uint32_t) = nullptr);
    ShaderHandle* LoadShader(const char* name);
    TextureHandle* LoadTexture(std::string name);
    PhysicsMaterialHandle* LoadPhysicsMaterial(const char* name);
    CubemapHandle* LoadCubemap(std::string name);
    AudioHandle* LoadAudio(const char* name);
}
