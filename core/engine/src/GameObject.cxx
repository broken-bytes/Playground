#include "playground/GameObject.hxx"
#include "playground/SceneManager.hxx"

namespace playground::gameobjects {
    Transform* GetGameObjectTransform(uint32_t id) {
        GameObject* go = scenemanager::GetGameObject(id);
        if (!go) {
            return nullptr;
        }

        return &go->transform;
    }
}
