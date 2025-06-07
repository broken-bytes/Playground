#include "playground/SceneManager.hxx"
#include <rendering/Rendering.hxx>
#include <assetloader/AssetLoader.hxx>

namespace playground::scenemanager {

    std::vector<GameObject*> _gameObjects;
    std::vector<uint32_t> _freeIds;

    void Init() {
        _gameObjects.clear();
        _freeIds.clear();
    }

    void Update() {
        for(auto& go : _gameObjects) {
            if (go == nullptr) {
                continue;
            }

            if (go->meshComponent != nullptr) {
                if (go->meshComponent->isUploaded) {
                    auto meshId = go->meshComponent->gpuMeshId;
                    auto materialId = go->meshComponent->gpuMaterialId;

                    // TODO: Create draw call for the mesh
                }
            }
        }
    }

    int32_t CreateGameObject() {
        auto go = new GameObject();
        go->transform = Transform();
        go->meshComponent = nullptr;
        go->audioSourceComponent = nullptr;

        go->transform.position = { 0.0f, 0.0f, 0.0f };
        go->transform.rotation = { 0.0f, 0.0f, 0.0f, 1.0f };
        go->transform.scale = { 1.0f, 1.0f, 1.0f };

        if (_freeIds.size() > 0) {
            int32_t id = _freeIds.back();
            _freeIds.pop_back();
            _gameObjects[id] = go;
            return id;
        } else {
            int32_t id = _gameObjects.size();
            _gameObjects.push_back(go);
            return id;
        }
    }

    void DestroyGameObject(int32_t id) {
        if (id < 0 || _gameObjects[id] == nullptr) {
            return;
        }

        delete _gameObjects[id]->meshComponent;
        delete _gameObjects[id]->audioSourceComponent;

        delete _gameObjects[id];
        _gameObjects[id] = nullptr;
        _freeIds.push_back(id);
    }

    GameObject* GetGameObject(int32_t id) {
        if (id < 0 || id >= _gameObjects.size()) {
            return nullptr;
        }

        return _gameObjects[id];
    }
}
