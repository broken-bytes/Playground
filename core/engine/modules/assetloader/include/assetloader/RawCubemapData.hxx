#pragma once

#include "assetloader/RawTextureData.hxx"
#include <array>
#include <map>
#include <string>
#include <vector>
#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/array.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/string.hpp>

namespace playground::assetloader
{
    struct RawCubemapData {
        enum class Format : uint8_t {
            Authoring = 0,
            Cooked = 1
        };

        ~RawCubemapData() {
            std::cout << "RawCubemapData destroyed" << std::endl;
        }

        uint16_t Width = 0;
        uint16_t Height = 0;
        uint8_t Channels = 0;
        std::vector<std::string> faces;
        std::vector<RawTextureData> facesData;
        std::vector<std::vector<std::vector<uint8_t>>> facesRawData;

        template <class Archive>
        void load(Archive& ar) {
            loadImpl(ar, cereal::traits::is_text_archive<Archive>());
        }

        template <class Archive>
        void save(Archive& ar) const {
            saveImpl(ar, cereal::traits::is_text_archive<Archive>());
        }

    private:
        template <class Archive>
        void loadImpl(Archive& ar, std::true_type) {
            // Text archive: assume authoring format
            ar(CEREAL_NVP(faces));
            Width = 0;
            Height = 0;
            Channels = 0;
            facesData.clear();
            facesRawData.clear();
        }

        template <class Archive>
        void loadImpl(Archive& ar, std::false_type) {
            // Binary archive: assume cooked
            ar(CEREAL_NVP(Width),
                CEREAL_NVP(Height),
                CEREAL_NVP(Channels),
                CEREAL_NVP(facesData));
            faces.clear();
            facesRawData.clear();
        }

        template <class Archive>
        void saveImpl(Archive& ar, std::true_type) const {
            // Save authoring format to JSON
            ar(CEREAL_NVP(faces));
        }

        template <class Archive>
        void saveImpl(Archive& ar, std::false_type) const {
            // Save cooked binary format
            ar(CEREAL_NVP(Width),
                CEREAL_NVP(Height),
                CEREAL_NVP(Channels),
                CEREAL_NVP(facesData));
        }
    };
}
