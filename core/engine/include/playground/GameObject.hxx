#pragma once

#include <assetloader/RawMeshData.hxx>
#include <array>
#include <cstdint>

namespace playground {
    struct Transform {
        std::array<float, 3> position;
        std::array<float, 4> rotation;
        std::array<float, 3> scale;
    };

    struct MeshComponent {
        bool isUploaded;
        std::string meshName;
        uint16_t subMeshIndex;
        uint32_t gpuMeshId;
        uint32_t gpuMaterialId;
        assetloader::RawMeshData* meshData;
    };

    struct AudioSourceComponent {
        uint32_t audioClipId;
        bool loop;
        float volume;
    };

    struct GameObject {
        uint32_t id;
        Transform transform;
        MeshComponent* meshComponent;
        AudioSourceComponent* audioSourceComponent;
    };


    namespace gameobjects {
        Transform* GetGameObjectTransform(uint32_t);
        MeshComponent* GetGameObjectMeshComponent(uint32_t);
        AudioSourceComponent* GetGameObjectAudioSourceComponent(uint32_t);
    }
}
