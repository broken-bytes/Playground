#include "playground/AssetManager.hxx"
#include <assetloader/AssetLoader.hxx>
#include <rendering/Mesh.hxx>
#include <rendering/Rendering.hxx>
#include <audio/Audio.hxx>
#include <physics/Physics.hxx>
#include <shared/Hasher.hxx>
#include <shared/JobSystem.hxx>
#include <io/IO.hxx>
#include <cstdint>
#include <ranges>
#include <string>
#include <vector>

namespace playground::assetmanager {
    std::vector<ModelHandle*> _modelHandles = {};
    std::vector<MaterialHandle*> _materialHandles = {};
    std::vector<ShaderHandle*> _shaderHandles = {};
    std::vector<TextureHandle*> _textureHandles = {};
    std::vector<PhysicsMaterialHandle*> _physicsMaterialHandles = {};
    std::vector<CubemapHandle*> _cubemapHandles = {};
    std::vector<AudioHandle*> _audioHandles = {};

    void MarkModelUploadFinished(uint32_t handleId, std::vector<rendering::Mesh> meshes) {
        if (handleId < _modelHandles.size()) {
            _modelHandles[handleId]->state.store(ResourceState::Uploaded);
            _modelHandles[handleId]->meshes = std::move(meshes);
            _modelHandles[handleId]->refCount--;
        }
    }

    void MarkMaterialUploadFinished(uint32_t handleId, uint32_t materialId) {
        if (handleId < _materialHandles.size()) {
            _materialHandles[handleId]->state.store(ResourceState::Uploaded);
            _materialHandles[handleId]->material = materialId;
            _materialHandles[handleId]->refCount--;

            int index = 0;
            for (auto& texture: _materialHandles[handleId]->textures) {
                rendering::SetMaterialTexture(materialId, index++, texture.second->texture);
                std::cout << "Material Texture: " << texture.second->texture << std::endl;
            }

            index = 0;
            for(auto& cubemap: _materialHandles[handleId]->cubemaps) {
                rendering::SetMaterialCubemap(materialId, index++, cubemap.second->cubemap);
                std::cout << "Material Cubemap: " << cubemap.second->cubemap << std::endl;
            }

            index = 0;
            for(auto floatProp: _materialHandles[handleId]->floats) {
                rendering::SetMaterialFloat(materialId, index++, floatProp.second);
                std::cout << "Material Float: " << floatProp.first << " = " << floatProp.second << std::endl;
            }

            if (_materialHandles[handleId]->onCompletion != nullptr) {
                _materialHandles[handleId]->onCompletion(materialId);
            }
        }
    }

    void MarkTextureUploadFinished(uint32_t handleId, uint32_t texture) {
        if (handleId < _textureHandles.size()) {
            _textureHandles[handleId]->state = ResourceState::Uploaded;
            _textureHandles[handleId]->data = nullptr;
            _textureHandles[handleId]->texture = texture;
            std::cout << "Texture: " << texture << std::endl;
            _textureHandles[handleId]->refCount--;
            delete _textureHandles[handleId]->data;

            jobsystem::Submit(_textureHandles[handleId]->signalUploadCompletionJob);
            _textureHandles[handleId]->signalUploadCompletionJob = nullptr;
        }
    }

    void MarkCubemapUploadFinished(uint32_t handleId, uint32_t cubemap) {
        if (handleId < _cubemapHandles.size()) {
            _cubemapHandles[handleId]->state = ResourceState::Uploaded;
            _cubemapHandles[handleId]->data = nullptr;
            _cubemapHandles[handleId]->faces = {};
            _cubemapHandles[handleId]->cubemap = cubemap;
            _cubemapHandles[handleId]->refCount--;

            jobsystem::Submit(_cubemapHandles[handleId]->signalUploadCompletionJob);
            _cubemapHandles[handleId]->signalUploadCompletionJob = nullptr;
        }
    }

    ModelHandle* LoadModel(const char* name) {
        auto hash = shared::Hash(name);

        ModelHandle* handle;
        for (uint32_t i = 0; i < _modelHandles.size(); ++i) {
            handle = _modelHandles[i];
            if (handle->hash == hash) {
                auto state = handle->state.load();
                if (state == ResourceState::Uploaded || state == ResourceState::Loading || state == ResourceState::Created) {
                    handle->refCount++;
                    return handle;
                }
                else if (state == ResourceState::Unloaded) {
                    auto rawMeshData = playground::assetloader::LoadMeshes(name);
                    handle->refCount = 1;
                    handle->state.store(ResourceState::Created);
                    playground::rendering::QueueUploadModel(rawMeshData, i, MarkModelUploadFinished);

                    return handle;
                }
            }
        }

        auto rawMeshData = playground::assetloader::LoadMeshes(name);

        auto newHandle = new ModelHandle {
            .hash = hash,
            .state = {ResourceState::Created},
            .refCount = 1,
            .meshes = {},
        };

        uint32_t handleId;
        handleId = _modelHandles.size();
        _modelHandles.push_back(std::move(newHandle));

        playground::rendering::QueueUploadModel(rawMeshData, handleId, MarkModelUploadFinished);

        return _modelHandles[handleId];
    }

    MaterialHandle* LoadMaterial(const char* name, void (*onCompletion)(uint32_t)) {
        auto hash = shared::Hash(name);

        std::optional<uint32_t> handleId;
        MaterialHandle* handle;
        for (uint32_t i = 0; i < _materialHandles.size(); ++i) {
            handle = _materialHandles[i];
            if (handle->hash == hash) {
                if (handle->state == ResourceState::Uploaded) {
                    handle->refCount++;

                    return handle;
                }
                else if (handle->state == ResourceState::Unloaded) {
                    handle = _materialHandles[i];
                    handleId = i;
                    handle->refCount = 1;
                    handle->state.store(ResourceState::Created);

                    break;
                }
            }
        }

        if (!handleId.has_value()) {
            handleId = _materialHandles.size();

            handle = new MaterialHandle{
              .hash = hash,
              .state = {ResourceState::Created},
              .refCount = 1,
              .material = 0,
              .onCompletion = onCompletion,
            };

            _materialHandles.push_back(handle);
        }

        auto materialName = std::string(name);
        auto rawMaterialData = playground::assetloader::LoadMaterial(materialName);

        auto materialLoadJob = jobsystem::JobHandle::Create(materialName + "_MATERIAL_UPLOAD_JOB", jobsystem::JobPriority::Low, [rawMaterialData, handleId, onCompletion]() {
            auto shader = playground::assetloader::LoadShader(rawMaterialData.shaderName);

            playground::rendering::MaterialType type;
            if (rawMaterialData.type == "skybox") {
                type = playground::rendering::MaterialType::Skybox;
            }
            else if (rawMaterialData.type == "standard") {
                type = playground::rendering::MaterialType::Standard;
            } else if (rawMaterialData.type == "shadow") {
                type = playground::rendering::MaterialType::Shadow;
            } else if (rawMaterialData.type == "PostProcessing") {
                type = playground::rendering::MaterialType::PostProcessing;
            } else {
                throw std::runtime_error("Unknown material type: " + rawMaterialData.type);
            }

            rendering::QueueUploadMaterial(
                shader.vertexShader,
                shader.pixelShader,
                type,
                handleId.value(),
                MarkMaterialUploadFinished,
                onCompletion
            );
        });

        std::vector<std::shared_ptr<jobsystem::JobHandle>> textureJobs;

        for (auto& texture : rawMaterialData.textures) {
            auto texHandle = LoadTexture(texture.value.c_str(), materialLoadJob.get());
            handle->textures.insert({ texture.name, texHandle });
        }

        for (auto& cubemap : rawMaterialData.cubemaps) {
            auto cubemapHandle = LoadCubemap(cubemap.value.c_str(), materialLoadJob.get());
            handle->cubemaps.insert({ cubemap.name, cubemapHandle });
        }

        // Get all float properties
        auto floatProps = rawMaterialData.props | std::views::filter([](const auto& prop) {
            return prop.type == "float";
        });

        for (auto& prop : floatProps) {
            handle->floats.insert({ prop.name, std::stof(prop.value) });
        }

        for (auto& textureJob : textureJobs) {
            materialLoadJob->AddDependency(textureJob);
        }

        for (auto& textureJob : textureJobs) {
            jobsystem::Submit(textureJob);
        }

        jobsystem::Submit(materialLoadJob);

        return _materialHandles[handleId.value()];
    }

    ShaderHandle* LoadShader(const char* name) {
        auto hash = shared::Hash(name);

        ShaderHandle* handle;
        for (uint32_t i = 0; i < _shaderHandles.size(); ++i) {
            handle = _shaderHandles[i];
            if (handle->hash == hash) {
                if (handle->state == ResourceState::Uploaded) {
                    handle->refCount++;
                    return handle;
                }
                else if (handle->state == ResourceState::Unloaded) {
                    auto rawShaderData = playground::assetloader::LoadShader(name);
                    handle->refCount = 1;
                    handle->state = ResourceState::Created;
                    handle->vertexShader = rawShaderData.vertexShader;
                    handle->pixelShader = rawShaderData.pixelShader;

                    return handle;
                }
            }
        }

        auto rawShaderData = playground::assetloader::LoadShader(name);

        auto newHandle = new ShaderHandle{
            .hash = hash,
            .state = ResourceState::Created,
            .refCount = 1,
            .vertexShader = rawShaderData.vertexShader,
            .pixelShader = rawShaderData.pixelShader,
        };

        uint32_t handleId;
        handleId = _shaderHandles.size();
        _shaderHandles.push_back(newHandle);

        return _shaderHandles[handleId];
    }

    TextureHandle* LoadTexture(const char* name, jobsystem::JobHandle* materialUploadJob) {
        auto hash = shared::Hash(name);

        std::optional<uint32_t> handleId;
        TextureHandle* handle;
        for (uint32_t i = 0; i < _textureHandles.size(); ++i) {
            handle = _textureHandles[i];
            if (handle->hash == hash) {
                if (handle->state.load() == ResourceState::Uploaded) {
                    handle->refCount++;

                    return handle;
                }
                else if (handle->state == ResourceState::Unloaded) {
                    auto rawTextureData = playground::assetloader::LoadTexture(name);
                    handle->refCount = 1;
                    handle->state.store(ResourceState::Created);
                }
            }
        }

        if (!handleId.has_value()) {
            handleId = _textureHandles.size();

            handle = new TextureHandle{
                .hash = hash,
                .state = {ResourceState::Created},
                .refCount = 1,
                .data = {}
            };

            _textureHandles.push_back(handle);
        }

        std::string textureName = std::string(name);

        handle->signalUploadCompletionJob = jobsystem::JobHandle::Create(
            textureName + "_TEXTURE_UPLOAD_JOB_COMPLETION",
            jobsystem::JobPriority::Low, [handle, handleId, textureName]() {
                // Dummy job, only signals texture readyness after GPU upload
        });

        auto textureLoadJob = jobsystem::JobHandle::Create(textureName + "_TEXTURE_UPLOAD_JOB", jobsystem::JobPriority::Low, [handle, handleId, textureName]() {
            auto rawTextureData = playground::assetloader::LoadTexture(textureName);
            auto data = new assetloader::RawTextureData();
            data->MipMaps = rawTextureData.MipMaps;
            data->Width = rawTextureData.Width;
            data->Height = rawTextureData.Height;
            data->Channels = rawTextureData.Channels;

            handle->data = data;

            rendering::QueueUploadTexture(handle->data, handleId.value(), &MarkTextureUploadFinished);
        });

        if (materialUploadJob != nullptr) {
            materialUploadJob->AddDependency(textureLoadJob);
            materialUploadJob->AddDependency(handle->signalUploadCompletionJob);
        }

        jobsystem::Submit(textureLoadJob);

        return _textureHandles[handleId.value()];
    }

    PhysicsMaterialHandle* LoadPhysicsMaterial(const char* name) {
        auto hash = shared::Hash(name);

        PhysicsMaterialHandle* handle;
        for (uint32_t i = 0; i < _physicsMaterialHandles.size(); ++i) {
            handle = _physicsMaterialHandles[i];
            if (handle->hash == hash) {
                if (handle->state == ResourceState::Uploaded) {
                    handle->refCount++;
                    return handle;
                }
                else if (handle->state == ResourceState::Unloaded) {
                    auto rawMaterial = playground::assetloader::LoadPhysicsMaterial(name);
                    auto rawHandle = playground::physics::CreateMaterial(rawMaterial.staticFriction, rawMaterial.dynamicFriction, rawMaterial.restitution);
                    handle->refCount = 1;
                    handle->state = ResourceState::Uploaded;
                    handle->material = rawHandle;

                    return handle;
                }
            }
        }

        auto rawMaterial = playground::assetloader::LoadPhysicsMaterial(name);
        auto rawHandle = playground::physics::CreateMaterial(rawMaterial.staticFriction, rawMaterial.dynamicFriction, rawMaterial.restitution);

        auto newHandle = new PhysicsMaterialHandle{
            .hash = hash,
            .state = ResourceState::Uploaded,
            .refCount = 1,
            .material = rawHandle
        };

        uint32_t handleId;
        handleId = _physicsMaterialHandles.size();
        _physicsMaterialHandles.push_back(newHandle);

        return _physicsMaterialHandles[handleId];
    }

    CubemapHandle* LoadCubemap(const char* name, jobsystem::JobHandle* materialUploadJob) {
        auto hash = shared::Hash(name);

        std::optional<uint32_t> handleId;
        CubemapHandle* handle;
        for (uint32_t i = 0; i < _cubemapHandles.size(); ++i) {
            handle = _cubemapHandles[i];
            if (handle->hash == hash) {
                if (handle->state.load() == ResourceState::Uploaded) {
                    handle->refCount++;

                    return handle;
                }
                else if (handle->state == ResourceState::Unloaded) {
                    handle->refCount = 1;
                    handle->state.store(ResourceState::Created);
                }
            }
        }

        if (!handleId.has_value()) {
            handleId = _cubemapHandles.size();

            handle = new CubemapHandle{
                .hash = hash,
                .state = {ResourceState::Created},
                .refCount = 1,
                .data = {},
                .faces = {},
                .signalUploadCompletionJob = nullptr,
                .cubemap = 0
            };

            _cubemapHandles.push_back(handle);
        }

        std::string cubemapName = std::string(name);

        handle->signalUploadCompletionJob = jobsystem::JobHandle::Create(
            cubemapName + "_CUBEMAP_UPLOAD_JOB_COMPLETION",
            jobsystem::JobPriority::Low, [handle, handleId, cubemapName]() {
                // Dummy job, only signals cubemap readyness after GPU upload
        });

        auto cubemapUploadJob = jobsystem::JobHandle::Create(cubemapName + "_CUBEMAP_UPLOAD_JOB", jobsystem::JobPriority::Low, [handle, handleId, cubemapName]() {
            auto rawCubemapData = playground::assetloader::LoadCubemap(cubemapName);
            auto data = std::make_shared<assetloader::RawCubemapData>(rawCubemapData);
            handle->data = data;

            rendering::QueueUploadCubemap(handle->data, handleId.value(), MarkCubemapUploadFinished);
        });

        if (materialUploadJob != nullptr) {
            materialUploadJob->AddDependency(cubemapUploadJob);
            materialUploadJob->AddDependency(handle->signalUploadCompletionJob);
        }

        jobsystem::Submit(cubemapUploadJob);

        return _cubemapHandles[handleId.value()];
    }

    AudioHandle* LoadAudio(const char* name) {
        auto hash = shared::Hash(name);
        AudioHandle* handle;
        for (uint32_t i = 0; i < _audioHandles.size(); ++i) {
            handle = _audioHandles[i];
            if (handle->hash == hash) {
                if (handle->state == ResourceState::Uploaded) {
                    handle->refCount++;
                    return handle;
                }
                else if (handle->state == ResourceState::Unloaded) {
                    auto archive = assetloader::TryFindFile(name);
                    if (archive.size() == 0) {
                        throw std::runtime_error("Audio file not found: " + std::string(name));
                    }
                    handle->audioBank = audio::LoadBank(archive, name);
                    handle->refCount = 1;
                    handle->state = ResourceState::Uploaded;

                    return handle;
                }
            }
        }
        handle = new AudioHandle{
            .hash = hash,
            .state = ResourceState::Created,
            .refCount = 1,
            .audioBank = nullptr
        };

        auto archive = assetloader::TryFindFile(name);
        if (archive.size() > 0) {
            handle->audioBank = audio::LoadBank(archive, name);
        } else {
            throw std::runtime_error("Audio file not found: " + std::string(name));
        }

        uint32_t handleId;
        handleId = _audioHandles.size();
        _audioHandles.push_back(handle);

        return _audioHandles[handleId];
    }
}
