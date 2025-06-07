#pragma once

#include "playground/GameObject.hxx"
#include <cstdint>
#include <vector>

namespace playground::scenemanager {
    void Init();
    void Update();
    int32_t CreateGameObject();
    void DestroyGameObject(int32_t id);
    GameObject* GetGameObject(int32_t id);
}
