#pragma once

#include "rendering/Material.hxx"
#include "rendering/Mesh.hxx"
#include <assetloader/RawMeshData.hxx>
#include <cstdint>
#include <string>
#include <functional>
#include <optional>
#include <vector>

namespace playground::rendering {
    struct MaterialUploadJob {
        uint32_t handle;
        MaterialType type;
        std::string vertexShaderBlob;
        std::string pixelShaderBlob;
        std::array<uint32_t, 8> textureHandles;
        std::array<uint32_t, 8> cubemapHandles;
        std::function<void(uint32_t, uint32_t)> callback;
        void (*onCompletion)(uint32_t);
    };
}
