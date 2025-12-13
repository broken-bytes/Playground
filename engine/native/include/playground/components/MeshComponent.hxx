#pragma once

#include <math/Vector3.hxx>
#include <cstdint>

namespace playground::assetmanager {
    struct ModelHandle;
}

struct MeshComponent {
    const char* name;
    playground::assetmanager::ModelHandle* handle;
    uint16_t meshId;
};

