#pragma once

#include <string>
#include <vector>

namespace playground::assetloader
{
    struct RawVertex {
        float x, y, z;
        float nx, ny, nz;
        float u, v;
    };

    struct RawMeshData {
        std::string name;
        std::vector<RawVertex> vertices;
        std::vector<uint32_t> indices;
    };
}
