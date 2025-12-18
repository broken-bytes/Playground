#pragma once

#include <cstdint>

namespace playground::assetmanager {
    struct ModelHandle;
}

struct MeshComponent {
    uint64_t AssetId;
};

struct MeshRuntimeComponent
{
    playground::assetmanager::ModelHandle* Handle;
    uint16_t MeshId;
};

