#include "playground/SceneManager.hxx"

namespace playground::scenemanager {
    std::vector<GameObject*> _gameObjects;
    std::vector<uint32_t> _freeIds;

    void Init() {
        _gameObjects.clear();
        _freeIds.clear();
    }

    int32_t CreateGameObject() {
        auto go = new GameObject();
        go->transform = Transform();

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

        delete _gameObjects[id];
        _gameObjects[id] = nullptr;
        _freeIds.push_back(id);
    }
}
