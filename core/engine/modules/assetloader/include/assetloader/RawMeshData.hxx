#pragma once

#include <string>
#include <vector>
#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>

namespace playground::assetloader
{
    struct RawVertex {
        float x, y, z;
        float nx, ny, nz;
        float u, v;

        template <class Archive>
        void serialize(Archive& ar)
        {
            ar(x, y, z, nx, ny, nz, u, v);
        }
    };

    struct RawMeshData {
        std::vector<RawVertex> vertices;
        std::vector<uint32_t> indices;

        template <class Archive>
        void serialize(Archive& ar)
        {
            ar(vertices, indices);
        }
    };
}
