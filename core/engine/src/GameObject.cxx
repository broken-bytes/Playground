#include "playground/GameObject.hxx"
#include "playground/AssetManager.hxx"
#include "playground/SceneManager.hxx"
#include <shared/Hasher.hxx>

namespace playground::gameobjects {
    GameObject* CreateGameObject() {
        auto go = new GameObject();
        go->transform = Transform();
        go->meshComponent = nullptr;
        go->audioSourceComponent = nullptr;

        go->transform.position = { 0.0f, 0.0f, 0.0f };
        go->transform.rotation = { 0.0f, 0.0f, 0.0f, 1.0f };
        go->transform.scale = { 1.0f, 1.0f, 1.0f };

        go->id = scenemanager::AddGameObject(go);

        return go;
    }

    void DestroyGameObject(GameObject* go) {
        scenemanager::DestroyGameObject(go->id);
    }

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

    MeshComponent* AddMeshComponent(uint32_t id, void* modelHandle, uint16_t meshId, void* materialHandle) {
        auto mesh = new MeshComponent();

        mesh->model = modelHandle;
        mesh->meshId = meshId;
        mesh->material = materialHandle;

        scenemanager::GetGameObject(id)->meshComponent = mesh;

        return mesh;
    }
}
