#include "playground/AssetManager.hxx"
#include <assetloader/AssetLoader.hxx>
#include <rendering/Mesh.hxx>
#include <rendering/Rendering.hxx>
#include <audio/Audio.hxx>
#include <physics/Physics.hxx>
#include <shared/Hasher.hxx>
#include <shared/Job.hxx>
#include <shared/JobHandle.hxx>
#include <shared/JobSystem.hxx>
#include <io/IO.hxx>
#include <cstdint>
#include <ranges>
#include <string>
#include <vector>
#include <tracy/Tracy.hpp>

namespace playground::assetmanager {
    std::vector<ModelHandle*> _modelHandles = {};
    std::vector<MaterialHandle*> _materialHandles = {};
    std::vector<ShaderHandle*> _shaderHandles = {};
    std::vector<TextureHandle*> _textureHandles = {};
    std::vector<PhysicsMaterialHandle*> _physicsMaterialHandles = {};
    std::vector<CubemapHandle*> _cubemapHandles = {};
    std::vector<AudioHandle*> _audioHandles = {};

    bool ParseU64(std::string_view s, uint64_t& out)
    {
        auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), out);
        return ec == std::errc{} && ptr == s.data() + s.size();
    }

    void MarkModelUploadFinished(uint32_t handleId, std::vector<rendering::Mesh> meshes) {
        if (handleId < _modelHandles.size()) {
            _modelHandles[handleId]->state.store(ResourceState::Uploaded);
            _modelHandles[handleId]->meshes = std::move(meshes);
            _modelHandles[handleId]->internalRefs--;
        }
    }

    void MarkMaterialUploadFinished(uint32_t handleId, uint32_t materialId) {
        auto materialSetupJob = jobsystem::Job{
            .Name = std::string("MATERIAL_SETUP_JOB_") + std::to_string(handleId),
            .Priority = jobsystem::JobPriority::Low,
            .Color = tracy::Color::Green,
            .Dependencies = {},
            .Task = [handleId, materialId](uint8_t workerId) {
                if (handleId < _materialHandles.size()) {
                    _materialHandles[handleId]->state.store(ResourceState::Uploaded);
                    _materialHandles[handleId]->material = materialId;
                    _materialHandles[handleId]->internalRefs--;

                    int index = 0;
                    for (auto& texture : _materialHandles[handleId]->textures) {
                        while (texture.second->completionMarkerJob == nullptr) {
                            std::this_thread::yield();
                        }
                        texture.second->completionMarkerJob->Wait();
                        rendering::SetMaterialTexture(materialId, index++, texture.second->texture);
                    }

                    index = 0;
                    for (auto& cubemap : _materialHandles[handleId]->cubemaps) {
                        while (cubemap.second->completionMarkerJob == nullptr) {
                            std::this_thread::yield();
                        }
                        cubemap.second->completionMarkerJob->Wait();
                        rendering::SetMaterialCubemap(materialId, index++, cubemap.second->cubemap);
                    }

                    index = 0;
                    for (auto floatProp : _materialHandles[handleId]->floats) {
                        rendering::SetMaterialFloat(materialId, index++, floatProp.second);
                    }

                    if (_materialHandles[handleId]->onCompletion != nullptr) {
                        _materialHandles[handleId]->onCompletion(materialId);
                    }
                }
            }
        };

        jobsystem::Submit(materialSetupJob);
    }

    void MarkTextureUploadFinished(uint32_t handleId, uint32_t texture) {
        if (handleId < _textureHandles.size()) {
            _textureHandles[handleId]->state = ResourceState::Uploaded;
            _textureHandles[handleId]->data = nullptr;
            _textureHandles[handleId]->texture = texture;
            _textureHandles[handleId]->internalRefs--;
            delete _textureHandles[handleId]->data;

            auto completion = jobsystem::Job{
                .Name = std::string("TEXTURE_UPLOAD_COMPLETION_") + std::to_string(handleId),
                .Priority = jobsystem::JobPriority::Low,
                .Color = tracy::Color::Violet,
                .Dependencies = {},
                .Task = [handleId](uint8_t workerId) {
                    // Additional completion logic can be added here if needed
                }
            };

            _textureHandles[handleId]->completionMarkerJob = jobsystem::Submit(completion);
        }
    }

    void MarkCubemapUploadFinished(uint32_t handleId, uint32_t cubemap) {
        if (handleId < _cubemapHandles.size()) {
            _cubemapHandles[handleId]->state = ResourceState::Uploaded;
            _cubemapHandles[handleId]->data = nullptr;
            _cubemapHandles[handleId]->faces = {};
            _cubemapHandles[handleId]->cubemap = cubemap;
            _cubemapHandles[handleId]->internalRefs--;

            auto completion = jobsystem::Job{
                .Name = std::string("CUBEMAP_UPLOAD_COMPLETION_") + std::to_string(handleId),
                .Priority = jobsystem::JobPriority::Low,
                .Color = tracy::Color::Violet,
                .Dependencies = {},
                .Task = [handleId](uint8_t workerId) {
                }
            };

            _cubemapHandles[handleId]->completionMarkerJob = jobsystem::Submit(completion);
        }
    }

    ModelHandle* LoadModel(uint64_t hash) {
        ModelHandle* handle;
        for (uint32_t i = 0; i < _modelHandles.size(); ++i) {
            handle = _modelHandles[i];
            if (handle->hash == hash) {
                auto state = handle->state.load();
                if (state == ResourceState::Uploaded || state == ResourceState::Loading || state == ResourceState::Created) {
                    handle->externalRefs++;
                    return handle;
                }
                if (state == ResourceState::Unloaded) {
                    auto rawMeshData = playground::assetloader::LoadMeshes(hash);
                    handle->externalRefs = 1;
                    handle->state.store(ResourceState::Created);
                    playground::rendering::QueueUploadModel(rawMeshData, i, MarkModelUploadFinished);

                    return handle;
                }
            }
        }

        auto rawMeshData = playground::assetloader::LoadMeshes(hash);

        auto newHandle = new ModelHandle{
            .hash = hash,
            .state = {ResourceState::Created},
            .externalRefs = 1,
            .internalRefs = 0,
            .meshes = {},
        };

        uint32_t handleId;
        handleId = _modelHandles.size();
        _modelHandles.push_back(std::move(newHandle));

        playground::rendering::QueueUploadModel(rawMeshData, handleId, MarkModelUploadFinished);

        return _modelHandles[handleId];
    }

    MaterialHandle* LoadMaterial(uint64_t hash, void (*onCompletion)(uint32_t)) {
        std::optional<uint32_t> handleId;
        MaterialHandle* handle;
        for (uint32_t i = 0; i < _materialHandles.size(); ++i) {
            handle = _materialHandles[i];
            if (handle->hash == hash) {
                if (handle->state == ResourceState::Uploaded) {
                    handle->externalRefs++;

                    return handle;
                }
                else if (handle->state == ResourceState::Unloaded) {
                    handle = _materialHandles[i];
                    handleId = i;
                    handle->externalRefs = 1;
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
                .externalRefs = 1,
                .internalRefs = 0,
                .material = 0,
                .onCompletion = onCompletion,
            };

            _materialHandles.push_back(handle);
        }

        auto rawMaterialData = playground::assetloader::LoadMaterial(hash);

        for (auto& texture : rawMaterialData.textures) {
            uint64_t hash;
            ParseU64(texture.value, hash);
            auto texHandle = LoadTexture(hash);
            handle->textures.insert({ texture.name, texHandle });
        }

        for (auto& cubemap : rawMaterialData.cubemaps) {
            uint64_t hash;
            ParseU64(cubemap.value, hash);
            auto cubemapHandle = LoadCubemap(hash);
            handle->cubemaps.insert({ cubemap.name, cubemapHandle });
        }

        // Get all float properties
        auto floatProps = rawMaterialData.props | std::views::filter([](const auto& prop) {
            return prop.type == "float";
            });

        for (auto& prop : floatProps) {
            handle->floats.insert({ prop.name, std::stof(prop.value) });
        }

        auto materialLoadJob = jobsystem::Job{
            .Name = "_MATERIAL_UPLOAD_JOB_" + std::to_string(handleId.value()),
            .Priority = jobsystem::JobPriority::Low,
            .Color = tracy::Color::Green,
            .Dependencies = {},
            .Task = [rawMaterialData, handleId, onCompletion](uint8_t workerId) {
                uint64_t hash;
                ParseU64(rawMaterialData.shaderName, hash);
                auto shader = playground::assetloader::LoadShader(hash);

                playground::rendering::MaterialType type;
                if (rawMaterialData.type == "skybox") {
                    type = playground::rendering::MaterialType::Skybox;
                }
                else if (rawMaterialData.type == "standard") {
                    type = playground::rendering::MaterialType::Standard;
                }
                else if (rawMaterialData.type == "shadow") {
                    type = playground::rendering::MaterialType::Shadow;
                }
                else if (rawMaterialData.type == "PostProcessing") {
                    type = playground::rendering::MaterialType::PostProcessing;
                }
                else {
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
            }
        };

        jobsystem::Submit(materialLoadJob);

        return _materialHandles[handleId.value()];
    }

    ShaderHandle* LoadShader(uint64_t hash) {
        ShaderHandle* handle;
        for (uint32_t i = 0; i < _shaderHandles.size(); ++i) {
            handle = _shaderHandles[i];
            if (handle->hash == hash) {
                if (handle->state == ResourceState::Uploaded) {
                    handle->externalRefs++;
                    return handle;
                }
                else if (handle->state == ResourceState::Unloaded) {
                    auto rawShaderData = playground::assetloader::LoadShader(hash);
                    handle->externalRefs = 1;
                    handle->state = ResourceState::Created;
                    handle->vertexShader = rawShaderData.vertexShader;
                    handle->pixelShader = rawShaderData.pixelShader;

                    return handle;
                }
            }
        }

        auto rawShaderData = playground::assetloader::LoadShader(hash);

        auto newHandle = new ShaderHandle{
            .hash = hash,
            .state = ResourceState::Created,
            .externalRefs = 1,
            .internalRefs = 0,
            .vertexShader = rawShaderData.vertexShader,
            .pixelShader = rawShaderData.pixelShader,
        };

        uint32_t handleId;
        handleId = _shaderHandles.size();
        _shaderHandles.push_back(newHandle);

        return _shaderHandles[handleId];
    }

    TextureHandle* LoadTexture(uint64_t hash) {
        std::optional<uint32_t> handleId;
        TextureHandle* handle;
        for (uint32_t i = 0; i < _textureHandles.size(); ++i) {
            handle = _textureHandles[i];
            if (handle->hash == hash) {
                if (handle->state.load() == ResourceState::Uploaded) {
                    handle->externalRefs++;

                    return handle;
                }
                else if (handle->state == ResourceState::Unloaded) {
                    auto rawTextureData = playground::assetloader::LoadTexture(hash);
                    handle->externalRefs = 1;
                    handle->state.store(ResourceState::Created);
                }
            }
        }

        if (!handleId.has_value()) {
            handleId = _textureHandles.size();

            handle = new TextureHandle{
                .hash = hash,
                .state = {ResourceState::Created},
                .externalRefs = 1,
                .internalRefs = 0,
                .data = {},
                .texture = 0,
                .uploadJob = nullptr,
                .completionMarkerJob = nullptr
            };

            _textureHandles.push_back(handle);
        }

        auto uploadJob = jobsystem::Job{
            .Name = "_TEXTURE_UPLOAD_JOB_" + std::to_string(hash),
            .Priority = jobsystem::JobPriority::Low,
            .Color = tracy::Color::Green,
            .Dependencies = {},
            .Task = [handle, handleId, hash](uint8_t workerId) {
                auto rawTextureData = playground::assetloader::LoadTexture(hash);
                auto data = new assetloader::RawTextureData();
                data->MipMaps = rawTextureData.MipMaps;
                data->Width = rawTextureData.Width;
                data->Height = rawTextureData.Height;
                data->Channels = rawTextureData.Channels;
                handle->data = data;
                rendering::QueueUploadTexture(handle->data, handleId.value(), MarkTextureUploadFinished);
            }
        };

        handle->uploadJob = jobsystem::Submit(uploadJob);

        return _textureHandles[handleId.value()];
    }

    PhysicsMaterialHandle* LoadPhysicsMaterial(uint64_t hash) {
        PhysicsMaterialHandle* handle;
        for (uint32_t i = 0; i < _physicsMaterialHandles.size(); ++i) {
            handle = _physicsMaterialHandles[i];
            if (handle->hash == hash) {
                if (handle->state == ResourceState::Uploaded) {
                    handle->externalRefs++;
                    return handle;
                }
                else if (handle->state == ResourceState::Unloaded) {
                    auto rawMaterial = playground::assetloader::LoadPhysicsMaterial(hash);
                    auto rawHandle = playground::physics::CreateMaterial(rawMaterial.staticFriction, rawMaterial.dynamicFriction, rawMaterial.restitution);
                    handle->externalRefs = 1;
                    handle->state = ResourceState::Uploaded;
                    handle->material = rawHandle;

                    return handle;
                }
            }
        }

        auto rawMaterial = playground::assetloader::LoadPhysicsMaterial(hash);
        auto rawHandle = playground::physics::CreateMaterial(rawMaterial.staticFriction, rawMaterial.dynamicFriction, rawMaterial.restitution);

        auto newHandle = new PhysicsMaterialHandle{
            .hash = hash,
            .state = ResourceState::Uploaded,
            .externalRefs = 1,
            .internalRefs = 0,
            .material = rawHandle
        };

        uint32_t handleId;
        handleId = _physicsMaterialHandles.size();
        _physicsMaterialHandles.push_back(newHandle);

        return _physicsMaterialHandles[handleId];
    }

    CubemapHandle* LoadCubemap(uint64_t hash) {
        std::optional<uint32_t> handleId;
        CubemapHandle* handle;
        for (uint32_t i = 0; i < _cubemapHandles.size(); ++i) {
            handle = _cubemapHandles[i];
            if (handle->hash == hash) {
                if (handle->state.load() == ResourceState::Uploaded) {
                    handle->externalRefs++;

                    return handle;
                }
                else if (handle->state == ResourceState::Unloaded) {
                    handle->externalRefs = 1;
                    handle->state.store(ResourceState::Created);
                }
            }
        }

        if (!handleId.has_value()) {
            handleId = _cubemapHandles.size();

            handle = new CubemapHandle{
                .hash = hash,
                .state = {ResourceState::Created},
                .externalRefs = 1,
                .internalRefs = 0,
                .data = {},
                .faces = {},
                .cubemap = 0,
                .uploadJob = nullptr,
                .completionMarkerJob = nullptr
            };

            _cubemapHandles.push_back(handle);
        }

        auto uploadJob = jobsystem::Job{
            .Name = "_CUBEMAP_UPLOAD_JOB_" + std::to_string(hash),
            .Priority = jobsystem::JobPriority::Low,
            .Color = tracy::Color::Blue,
            .Dependencies = {},
            .Task = [handle, handleId, hash](uint8_t workerId) {
                auto rawCubemapData = playground::assetloader::LoadCubemap(hash);
                auto data = std::make_shared<assetloader::RawCubemapData>(rawCubemapData);
                handle->data = data;
                rendering::QueueUploadCubemap(handle->data, handleId.value(), MarkCubemapUploadFinished);
            }
        };

         handle->uploadJob = jobsystem::Submit(uploadJob);

        return _cubemapHandles[handleId.value()];
    }

    AudioHandle* LoadAudio(uint64_t hash, std::string name) {
        AudioHandle* handle;
        for (uint32_t i = 0; i < _audioHandles.size(); ++i) {
            handle = _audioHandles[i];
            if (handle->hash == hash) {
                if (handle->state == ResourceState::Uploaded) {
                    handle->externalRefs++;
                    return handle;
                }
                else if (handle->state == ResourceState::Unloaded) {
                    auto archive = assetloader::TryFindFile(hash);
                    if (archive.size() == 0) {
                        throw std::runtime_error("Audio file not found: " + std::to_string(hash));
                    }
                    handle->audioBank = audio::LoadBank(archive, name);
                    handle->externalRefs = 1;
                    handle->state = ResourceState::Uploaded;

                    return handle;
                }
            }
        }
        handle = new AudioHandle{
            .hash = hash,
            .state = ResourceState::Created,
            .externalRefs = 1,
            .internalRefs = 0,
            .audioBank = nullptr
        };

        auto archive = assetloader::TryFindFile(hash);
        if (archive.size() > 0) {
            handle->audioBank = audio::LoadBank(archive, name);
        }
        else {
            throw std::runtime_error("Audio file not found: " + std::to_string(hash));
        }

        uint32_t handleId;
        handleId = _audioHandles.size();
        _audioHandles.push_back(handle);

        return _audioHandles[handleId];
    }

    void LoadSceneData(uint64_t hash, char* data, size_t* size)
    {
        auto archive = assetloader::TryFindFile(hash);
        if (archive.size() == 0) {
            throw std::runtime_error("Scene file not found: " + std::to_string(hash));
        }

        auto fileData = assetloader::TryLoadFile(hash);

        if (data == nullptr)
        {
            *size = fileData.size();
            return;
        }

        memcpy(data, fileData.data(), fileData.size());
        data[fileData.size()] = '\0';

        *size = fileData.size();
    }

    ModelHandle* LoadModelByName(const char* name)
    {
        auto hash = shared::Hash(name);

        return LoadModel(hash);
    }

    MaterialHandle* LoadMaterialByName(const char* name, void (*onCompletion)(uint32_t))
    {
        auto hash = shared::Hash(name);

        return LoadMaterial(hash, onCompletion);
    }

    ShaderHandle* LoadShaderByName(const char* name)
    {
        auto hash = shared::Hash(name);

        return LoadShader(hash);
    }

    TextureHandle* LoadTextureByName(const char* name)
    {
        auto hash = shared::Hash(name);

        return LoadTexture(hash);
    }

    PhysicsMaterialHandle* LoadPhysicsMaterialByName(const char* name)
    {
        auto hash = shared::Hash(name);

        return LoadPhysicsMaterial(hash);
    }

    CubemapHandle* LoadCubemapByName(const char* name)
    {
        auto hash = shared::Hash(name);

        return LoadCubemap(hash);
    }

    void LoadSceneDataByName(const char* name, char* data, size_t* size)
    {
        auto hash = shared::Hash(name);

        LoadSceneData(hash, data, size);
    }

    void ReleaseModel(ModelHandle* handle)
    {
        handle->externalRefs--;
        // TODO: Free model
    }

    void ReleaseMaterial(MaterialHandle* handle)
    {

    }

    void ReleaseShader(ShaderHandle* handle)
    {

    }

    void ReleaseTexture(TextureHandle* handle)
    {

    }

    void ReleaseCubemap(CubemapHandle* handle)
    {

    }

    void ReleaseAudio(AudioHandle* handle)
    {

    }

    void ReleasePhysicsMaterial(PhysicsMaterialHandle* handle)
    {

    }
}
