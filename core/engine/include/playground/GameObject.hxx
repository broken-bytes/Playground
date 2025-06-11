#pragma once

#include "playground/AssetManager.hxx"
#include <assetloader/RawMeshData.hxx>
#include <array>
#include <cstdint>
#include <string_view>

namespace playground {
    struct Transform {
        std::array<float, 3> position;
        std::array<float, 4> rotation;
        std::array<float, 3> scale;
    };

    struct MeshComponent {
        std::string modelName;
        uint16_t subMeshIndex;
        std::string materialName;
        assetloader::RawMeshData* meshData;
    };

    struct AudioSourceComponent {
        uint32_t audioClipId;
        bool loop;
        float volume;
    };

    struct GameObject {
        uint32_t id;
        uint32_t parentId;
        std::vector<uint32_t> children;
        Transform transform;
        MeshComponent* meshComponent;
        AudioSourceComponent* audioSourceComponent;
    };

    namespace gameobjects {
        Transform* GetGameObjectTransform(uint32_t);
        MeshComponent* AddMeshComponent(uint32_t id, const char* modelName, uint16_t subMeshIndex, const char* materialName);
        MeshComponent* GetGameObjectMeshComponent(uint32_t);
        AudioSourceComponent* GetGameObjectAudioSourceComponent(uint32_t);
    }
}
