#pragma once

#include <cstdint>
#include <vector>
#include <cereal/cereal.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/vector.hpp>

namespace playground::assetloader {
    struct RawAudioData {
        std::vector<uint8_t> bankData;

        template <class Archive>
        void serialize(Archive& ar)
        {
            ar(bankData);
        }
    };
}
