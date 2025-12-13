#pragma once

#include "rendering/Mesh.hxx"
#include <assetloader/RawMeshData.hxx>
#include <cstdint>
#include <functional>
#include <vector>

namespace playground::rendering {
    struct ModelUploadJob {
        uint32_t handle;
        std::vector<assetloader::RawMeshData> meshes;
        std::function<void(uint32_t, std::vector<Mesh>)> callback;
    };
}
