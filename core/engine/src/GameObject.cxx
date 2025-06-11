#include "playground/GameObject.hxx"
#include "playground/AssetManager.hxx"
#include "playground/SceneManager.hxx"
#include <shared/Hasher.hxx>

namespace playground::gameobjects {
    Transform* GetGameObjectTransform(uint32_t id) {
        GameObject* go = scenemanager::GetGameObject(id);
        if (!go) {
            return nullptr;
        }

        return &go->transform;
    }

    MeshComponent* GetGameObjectMeshComponent(uint32_t id) {
        auto go = scenemanager::GetGameObject(id);

        if (!go) {
            return nullptr;
        }

        return go->meshComponent;
    }

    AudioSourceComponent* GetGameObjectAudioSourceComponent(uint32_t id) {
        auto go = scenemanager::GetGameObject(id);
        if (!go) {
            return nullptr;
        }

        return go->audioSourceComponent;
    }

    MeshComponent* AddMeshComponent(uint32_t id, const char* modelName, uint16_t subMeshIndex, const char* materialName) {
        auto mesh = new MeshComponent();

        mesh->modelName = modelName;
        mesh->subMeshIndex = subMeshIndex;
        mesh->materialName = materialName;
        scenemanager::GetGameObject(id)->meshComponent = mesh;

        return mesh;
    }
}
