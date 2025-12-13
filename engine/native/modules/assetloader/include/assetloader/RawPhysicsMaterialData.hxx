#pragma once

#include <array>
#include <map>
#include <string>
#include <stdexcept>
#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/string.hpp>

namespace playground::assetloader
{
    struct RawPhysicsMaterialData {
        std::string name;
        float staticFriction;
        float dynamicFriction;
        float restitution;

        template <class Archive>
        void serialize(Archive& ar)
        {
            ar(
                CEREAL_NVP(name),
                CEREAL_NVP(staticFriction),
                CEREAL_NVP(dynamicFriction),
                CEREAL_NVP(restitution)
            );
        }
    };

}
