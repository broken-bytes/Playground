#include "playground/AssetManager.hxx"
#include <assetloader/AssetLoader.hxx>
#include <rendering/Mesh.hxx>
#include <cstdint>
#include <string>
#include <vector>

namespace playground::assetmanager {
    std::vector<ModelHandle> _modelHandles = {};
    std::vector<uint32_t> _freeModelHandleIds = {};

    uint32_t LoadModel(const char* name, uint16_t index) {
        std::string meshName(name);

        for (uint32_t i = 0; i < _modelHandles.size(); ++i) {
            if (_modelHandles[i].name == meshName) {
                _modelHandles[i].refCount++;
                return i;
            }
        }

        auto rawMeshData = playground::assetloader::LoadMeshes(meshName);

        auto handle = ModelHandle {
            .isUploaded = false,
            .name = meshName,
            .refCount = 1,
            .meshes = {},
            .rawMeshData = rawMeshData,
        };

        uint32_t handleId;
        if (!_freeModelHandleIds.empty()) {
            handleId = _freeModelHandleIds.back();
            _freeModelHandleIds.pop_back();
            _modelHandles[handleId] = handle;
        }
        else {
            handleId = _modelHandles.size();
            _modelHandles.push_back(handle);
        }

        return handleId;
    }
}
