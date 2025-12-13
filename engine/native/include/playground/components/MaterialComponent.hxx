#pragma once

#include <math/Vector3.hxx>
#include <cstdint>

namespace playground::assetmanager {
    struct MaterialHandle;
}

struct MaterialComponent {
    const char* name;
    playground::assetmanager::MaterialHandle* handle;
};

