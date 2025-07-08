#include "playground/AssetManager.hxx"
#include <assetloader/AssetLoader.hxx>
#include <rendering/Mesh.hxx>
#include <rendering/Rendering.hxx>
#include <physics/Physics.hxx>
#include <shared/Hasher.hxx>
#include <cstdint>
#include <string>
#include <vector>

namespace playground::assetmanager {
    std::vector<ModelHandle*> _modelHandles = {};
    std::vector<MaterialHandle*> _materialHandles = {};
    std::vector<ShaderHandle*> _shaderHandles = {};
    std::vector<TextureHandle*> _textureHandles = {};
    std::vector<PhysicsMaterialHandle*> _physicsMaterialHandles = {};

    void MarkModelUploadFinished(uint32_t handleId, std::vector<rendering::Mesh> meshes) {
        if (handleId < _modelHandles.size()) {
            _modelHandles[handleId]->state = ResourceState::Uploaded;
            _modelHandles[handleId]->meshes = std::move(meshes);
            _modelHandles[handleId]->refCount--;
        }
    }

    void MarkMaterialUploadFinished(uint32_t handleId, uint32_t materialId) {
        if (handleId < _materialHandles.size()) {
            _materialHandles[handleId]->state = ResourceState::Uploaded;
            _materialHandles[handleId]->material = materialId;
            _materialHandles[handleId]->refCount--;
        }
    }

    void MarkTextureUploadFinished(uint32_t handleId, uint32_t texture) {
        if (handleId < _textureHandles.size()) {
            _textureHandles[handleId]->state = ResourceState::Uploaded;
            _textureHandles[handleId]->data = nullptr;
            _textureHandles[handleId]->texture = texture;
            _textureHandles[handleId]->refCount--;
        }
    }

    ModelHandle* LoadModel(const char* name) {
        auto hash = shared::Hash(name);

        ModelHandle* handle;
        for (uint32_t i = 0; i < _modelHandles.size(); ++i) {
            handle = _modelHandles[i];
            if (handle->hash == hash) {
                if (handle->state == ResourceState::Uploaded || handle->state == ResourceState::Loading || handle->state == ResourceState::Created) {
                    handle->refCount++;
                    return handle;
                }
                else if (handle->state == ResourceState::Unloaded) {
                    auto rawMeshData = playground::assetloader::LoadMeshes(name);
                    handle->refCount = 1;
                    handle->state = ResourceState::Created;
                    playground::rendering::QueueUploadModel(rawMeshData, i, MarkModelUploadFinished);

                    return handle;
                }
            }
        }

        auto rawMeshData = playground::assetloader::LoadMeshes(name);

        auto newHandle = new ModelHandle {
            .hash = hash,
            .state = ResourceState::Created,
            .refCount = 1,
            .meshes = {},
        };

        uint32_t handleId;
        handleId = _modelHandles.size();
        _modelHandles.push_back(std::move(newHandle));

        playground::rendering::QueueUploadModel(rawMeshData, handleId, MarkModelUploadFinished);

        return _modelHandles[handleId];
    }

    MaterialHandle* LoadMaterial(const char* name) {
        auto hash = shared::Hash(name);

        MaterialHandle* handle;
        for (uint32_t i = 0; i < _materialHandles.size(); ++i) {
            handle = _materialHandles[i];
            if (handle->hash == hash) {
                if (handle->state == ResourceState::Uploaded) {
                    handle->refCount++;
                    return handle;
                }
                else if (handle->state == ResourceState::Unloaded) {
                    auto rawMaterialData = playground::assetloader::LoadMaterial(name);
                    handle->refCount = 1;
                    handle->state = ResourceState::Created;

                    auto shader = assetloader::LoadShader(rawMaterialData.shaderName);

                    rendering::QueueUploadMaterial(
                        shader.vertexShader,
                        shader.pixelShader,
                        i,
                        MarkMaterialUploadFinished
                    );

                    return handle;
                }
            }
        }

        auto rawMaterialData = playground::assetloader::LoadMaterial(name);
        auto shader = playground::assetloader::LoadShader(rawMaterialData.shaderName);

        for (auto& texture : rawMaterialData.textures) {
            LoadTexture(texture.value.c_str());
        }

        auto newHandle = new MaterialHandle {
            .hash = hash,
            .state = ResourceState::Created,
            .refCount = 1,
            .material = 0,
        };

        uint32_t handleId;
        handleId = _materialHandles.size();
        _materialHandles.push_back(newHandle);

        rendering::QueueUploadMaterial(
            shader.vertexShader,
            shader.pixelShader,
            handleId,
            MarkMaterialUploadFinished
        );

        return _materialHandles[handleId];
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

    TextureHandle* LoadTexture(const char* name) {
        auto hash = shared::Hash(name);

        TextureHandle* handle;
        for (uint32_t i = 0; i < _textureHandles.size(); ++i) {
            handle = _textureHandles[i];
            if (handle->hash == hash) {
                if (handle->state == ResourceState::Uploaded) {
                    handle->refCount++;
                    return handle;
                }
                else if (handle->state == ResourceState::Unloaded) {
                    auto rawTextureData = playground::assetloader::LoadTexture(name);
                    handle->refCount = 1;
                    handle->state = ResourceState::Created;
                    handle->data = std::make_shared<assetloader::RawTextureData>(rawTextureData);

                    return handle;
                }
            }
        }

        auto rawTextureData = playground::assetloader::LoadTexture(name);

        auto data = std::make_shared<assetloader::RawTextureData>(rawTextureData);

        auto newHandle = new TextureHandle{
            .hash = hash,
            .state = ResourceState::Created,
            .refCount = 1,
            .data = data
        };

        uint32_t handleId;
        handleId = _textureHandles.size();
        _textureHandles.push_back(newHandle);

        rendering::QueueUploadTexture(data, handleId, MarkTextureUploadFinished);

        return _textureHandles[handleId];
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
}
