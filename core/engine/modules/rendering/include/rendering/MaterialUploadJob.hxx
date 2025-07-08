#pragma once

#include "rendering/Mesh.hxx"
#include <assetloader/RawMeshData.hxx>
#include <cstdint>
#include <string>
#include <functional>
#include <vector>

namespace playground::rendering {
    struct MaterialUploadJob {
        uint32_t handle;
        std::string vertexShaderBlob;
        std::string pixelShaderBlob;
        std::array<uint32_t, 8> textureHandles;
        std::function<void(uint32_t, uint32_t)> callback;
    };
}
