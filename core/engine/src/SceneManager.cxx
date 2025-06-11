#include "playground/SceneManager.hxx"
#include "playground/AssetManager.hxx"
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

            assetmanager::ModelHandle* modelToDraw = 0;
            assetmanager::MaterialHandle* materialToDraw = 0;

            if (go->meshComponent != nullptr) {
                modelToDraw = assetmanager::LoadModel(go->meshComponent->modelName);
                if (modelToDraw->state != assetmanager::ResourceState::Uploaded) {
                    continue;
                }

                materialToDraw = assetmanager::LoadMaterial(go->meshComponent->materialName);

                if (materialToDraw->state != assetmanager::ResourceState::Uploaded) {
                    continue;
                }

                auto mesh = modelToDraw->meshes[go->meshComponent->subMeshIndex];
                auto material = materialToDraw->material;;

                rendering::DrawIndexed(mesh.vertexBuffer, mesh.indexBuffer, material);
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
