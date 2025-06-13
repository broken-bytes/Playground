#pragma once

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
        void* model;
        uint16_t meshId;
        void* material;
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
        GameObject* CreateGameObject();
        void DestroyGameObject(GameObject* go);
        Transform* GetGameObjectTransform(uint32_t);
        MeshComponent* AddMeshComponent(uint32_t id, void* modelHandle, uint16_t meshId, void* materialHandle);
        MeshComponent* GetGameObjectMeshComponent(uint32_t);
        AudioSourceComponent* GetGameObjectAudioSourceComponent(uint32_t);
    }
}
