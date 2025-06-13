#include "playground/AssetManager.hxx"
#include <assetloader/AssetLoader.hxx>
#include <rendering/Mesh.hxx>
#include <rendering/Rendering.hxx>
#include <shared/Hasher.hxx>
#include <cstdint>
#include <string>
#include <vector>

namespace playground::assetmanager {
    std::vector<ModelHandle*> _modelHandles = {};
    std::vector<MaterialHandle*> _materialHandles = {};
    std::vector<ShaderHandle*> _shaderHandles = {};

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

                    auto vertexShader = assetloader::LoadShader(rawMaterialData.vertexShaderName);
                    auto pixelShader = assetloader::LoadShader(rawMaterialData.pixelShaderName);

                    rendering::QueueUploadMaterial(
                        vertexShader.blob,
                        pixelShader.blob,
                        i,
                        MarkMaterialUploadFinished
                    );

                    return handle;
                }
            }
        }

        auto rawMaterialData = playground::assetloader::LoadMaterial(name);
        auto vertexShader = playground::assetloader::LoadShader(rawMaterialData.vertexShaderName);
        auto pixelShader = playground::assetloader::LoadShader(rawMaterialData.pixelShaderName);

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
            vertexShader.blob,
            pixelShader.blob,
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
                    handle->bytecode = rawShaderData.blob;

                    return handle;
                }
            }
        }

        auto rawShaderData = playground::assetloader::LoadShader(name);

        auto newHandle = new ShaderHandle{
            .hash = hash,
            .state = ResourceState::Created,
            .refCount = 1,
            .bytecode = rawShaderData.blob
        };

        uint32_t handleId;
        handleId = _shaderHandles.size();
        _shaderHandles.push_back(newHandle);

        return _shaderHandles[handleId];
    }
}
