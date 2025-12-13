#pragma once

#include <math/Vector3.hxx>
#include <math/Quaternion.hxx>
#include <string>
#include <vector>
#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>

namespace playground::assetloader
{
    struct RawBoneVertexWeight {
        int bone;
        float weight;

        template <class Archive>
        void serialize(Archive& ar)
        {
            ar(bone, weight);
        }
    };

    struct RawBone {
        std::string name;
        int parent;
        float tx, ty, tz;
        float rx, ry, rz, rw;
        float sx, sy, sz;
        std::vector<RawBoneVertexWeight> vertexWeights;

        template <class Archive>
        void serialize(Archive& ar)
        {
            ar(name, parent, tx, ty, tz, rx, ry, rz, rw, sx, sy, sz);
        }
    };

    struct RawVertex {
        float x, y, z;
        float cr, cg, cb, ca;
        float nx, ny, nz;
        float u, v;

        template <class Archive>
        void serialize(Archive& ar)
        {
            ar(x, y, z, cr, cg, cb, ca, nx, ny, nz, u, v);
        }
    };

    struct RawMeshData {
        float posX;
        float posY;
        float posZ;
        float rotX;
        float rotY;
        float rotZ;
        float rotW;
        float scaleX;
        float scaleY;
        float scaleZ;
        std::string name;
        std::vector<RawBone> bones;
        std::vector<RawVertex> vertices;
        std::vector<uint32_t> indices;

        template <class Archive>
        void serialize(Archive& ar)
        {
            ar(posX, posY, posZ, rotX, rotY, rotZ, rotW, scaleX, scaleY, scaleZ, name, vertices, indices);
        }
    };
}
