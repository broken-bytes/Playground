#pragma once

#include <cstdint>

namespace playground::assetmanager {
    struct ModelHandle;
}

struct alignas(8) MeshComponent {
    uint64_t AssetId;
    uint16_t MeshId;
};

struct MeshRuntimeComponent
{
    uint32_t HandleId;
    uint16_t MeshId;
};

