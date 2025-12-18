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

    ModelHandle* GetModel(uint32_t handle);
    TextureHandle* GetTexture(uint32_t handle);
    CubemapHandle* GetCubemap(uint32_t handle);
    MaterialHandle* GetMaterial(uint32_t handle);
    ShaderHandle* GetShader(uint32_t handle);
    AudioHandle* GetAudio(uint32_t handle);
    PhysicsMaterialHandle* GetPhysicsMaterial(uint32_t handle);

    uint32_t LoadModel(uint64_t hash);
    uint32_t LoadMaterial(uint64_t hash, void (*onCompletion)(uint32_t) = nullptr);
    uint32_t LoadShader(uint64_t hash);
    uint32_t LoadTexture(uint64_t hash);
    uint32_t LoadPhysicsMaterial(uint64_t hash);
    uint32_t LoadCubemap(uint64_t hash);
    uint32_t LoadAudio(uint64_t hash, std::string name);
    void LoadSceneData(uint64_t hash, char* data, size_t* size);

    uint32_t LoadModelByName(const char* name);
    uint32_t LoadMaterialByName(const char* name, void (*onCompletion)(uint32_t) = nullptr);
    uint32_t LoadShaderByName(const char* name);
    uint32_t LoadTextureByName(const char* name);
    uint32_t LoadPhysicsMaterialByName(const char* name);
    uint32_t LoadCubemapByName(const char* name);
    void LoadSceneDataByName(const char* name, char* data, size_t* size);

    void ReleaseModel(uint32_t handle);
    void ReleaseMaterial(uint32_t handle);
    void ReleaseShader(uint32_t handle);
    void ReleaseTexture(uint32_t handle);
    void ReleaseCubemap(uint32_t handle);
    void ReleaseAudio(uint32_t handle);
    void ReleasePhysicsMaterial(uint32_t handle);
}
