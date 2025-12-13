#pragma once

#include <string>
#include <vector>
#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>

namespace playground::assetloader
{
    struct RawKeyframe {
        float time;  // The time at which this keyframe happens
        float px, py, pz;  // For position keyframes (x, y, z)
        float rx, ry, rz, rw;  // For rotation keyframes (x, y, z, w)
        float sx, sy, sz;  // For scaling keyframes (x, y, z)

        template <class Archive>
        void serialize(Archive& ar)
        {
            ar(time, px, py, pz, rx, ry, rz, rw, sx, sy, sz);
        }
    };

    struct RawAnimationChannel {
        std::string nodeName;  // Name of the node this channel affects
        std::vector<RawKeyframe> positionKeys;  // Position keyframes
        std::vector<RawKeyframe> rotationKeys;  // Rotation keyframes
        std::vector<RawKeyframe> scalingKeys;   // Scaling keyframes

        template <class Archive>
        void serialize(Archive& ar)
        {
            ar(nodeName, positionKeys, rotationKeys, scalingKeys);
        }
    };

    struct RawAnimationData {
        std::string name;
        float duration;
        int framesPerSecond;
        std::vector<RawAnimationChannel> channels;  // Animation channels


        template <class Archive>
        void serialize(Archive& ar)
        {
            ar(name, duration, framesPerSecond, channels);
        }
    };
}
