#pragma once

#include <cstdint>
#include <vector>
#include <cereal/cereal.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/vector.hpp>

namespace playground::assetloader {
    struct RawSceneData {
        std::vector<uint8_t> sceneData;

        template <class Archive>
        void serialize(Archive& ar)
        {
            ar(sceneData);
        }
    };
}
