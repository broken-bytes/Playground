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

    ModelHandle* LoadModel(const char* name);
    MaterialHandle* LoadMaterial(const char* name, void (*onCompletion)(uint32_t) = nullptr);
    ShaderHandle* LoadShader(const char* name);
    TextureHandle* LoadTexture(const char* name);
    PhysicsMaterialHandle* LoadPhysicsMaterial(const char* name);
    CubemapHandle* LoadCubemap(const char* name);
    AudioHandle* LoadAudio(const char* name);

    void ReleaseModel(ModelHandle* handle);
    void ReleaseMaterial(MaterialHandle* handle);
    void ReleaseShader(ShaderHandle* handle);
    void ReleaseTexture(TextureHandle* handle);
    void ReleaseCubemap(CubemapHandle* handle);
    void ReleaseAudio(AudioHandle* handle);
    void ReleasePhysicsMaterial(PhysicsMaterialHandle* handle);
}
