#pragma once

#include <math/Vector3.hxx>
#include <cstdint>

namespace playground::assetmanager {
    struct MaterialHandle;
}

struct MaterialComponent {
    playground::assetmanager::MaterialHandle* handle;
};

