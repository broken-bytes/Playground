#pragma once

#include <cstdint>

namespace playground::assetmanager {
    struct ModelHandle;
}

struct MeshComponent {
    playground::assetmanager::ModelHandle* handle;
    uint16_t meshId;
};

