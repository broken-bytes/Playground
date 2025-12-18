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
        std::atomic<uint32_t> externalRefs; // ECS/components
        std::atomic<uint32_t> internalRefs;
        std::vector<playground::rendering::Mesh> meshes;
        std::vector<assetloader::RawMeshData> rawMeshData;
    };

    struct TextureHandle {
        uint64_t hash;
        std::atomic<ResourceState> state;
        std::atomic<uint32_t> externalRefs; // ECS/components
        std::atomic<uint32_t> internalRefs;
        assetloader::RawTextureData* data;
        uint32_t texture;
        std::shared_ptr<jobsystem::JobHandle> uploadJob;
        std::shared_ptr<jobsystem::JobHandle> completionMarkerJob;
    };

    struct CubemapHandle {
        uint64_t hash;
        std::atomic<ResourceState> state;
        std::atomic<uint32_t> externalRefs; // ECS/components
        std::atomic<uint32_t> internalRefs;
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
        std::atomic<uint32_t> externalRefs; // ECS/components
        std::atomic<uint32_t> internalRefs;
        uint32_t material;
        void (*onCompletion)(uint32_t);
    };

    struct ShaderHandle {
        uint64_t hash;
        ResourceState state;
        std::atomic<uint32_t> externalRefs; // ECS/components
        std::atomic<uint32_t> internalRefs;
        std::string vertexShader;
        std::string pixelShader;
    };

    struct AudioHandle {
        uint64_t hash;
        ResourceState state;
        std::atomic<uint32_t> externalRefs; // ECS/components
        std::atomic<uint32_t> internalRefs;
        void* audioBank;
    };

    struct PhysicsMaterialHandle {
        uint64_t hash;
        ResourceState state;
        std::atomic<uint32_t> externalRefs; // ECS/components
        std::atomic<uint32_t> internalRefs;
        uint32_t material;
    };

    ModelHandle* LoadModel(uint64_t hash);
    MaterialHandle* LoadMaterial(uint64_t hash, void (*onCompletion)(uint32_t) = nullptr);
    ShaderHandle* LoadShader(uint64_t hash);
    TextureHandle* LoadTexture(uint64_t hash);
    PhysicsMaterialHandle* LoadPhysicsMaterial(uint64_t hash);
    CubemapHandle* LoadCubemap(uint64_t hash);
    AudioHandle* LoadAudio(uint64_t hash, std::string name);
    void LoadSceneData(uint64_t hash, char* data, size_t* size);

    ModelHandle* LoadModelByName(const char* name);
    MaterialHandle* LoadMaterialByName(const char* name, void (*onCompletion)(uint32_t) = nullptr);
    ShaderHandle* LoadShaderByName(const char* name);
    TextureHandle* LoadTextureByName(const char* name);
    PhysicsMaterialHandle* LoadPhysicsMaterialByName(const char* name);
    CubemapHandle* LoadCubemapByName(const char* name);
    void LoadSceneDataByName(const char* name, char* data, size_t* size);

    void ReleaseModel(ModelHandle* handle);
    void ReleaseMaterial(MaterialHandle* handle);
    void ReleaseShader(ShaderHandle* handle);
    void ReleaseTexture(TextureHandle* handle);
    void ReleaseCubemap(CubemapHandle* handle);
    void ReleaseAudio(AudioHandle* handle);
    void ReleasePhysicsMaterial(PhysicsMaterialHandle* handle);
}
