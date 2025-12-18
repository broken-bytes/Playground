#pragma once

#include <math/Vector3.hxx>
#include <cstdint>

namespace playground::assetmanager {
    struct MaterialHandle;
}

struct MaterialComponent {
    uint64_t AssetId;
};

struct MaterialRuntimeComponent {
    playground::assetmanager::MaterialHandle* handle;
};


