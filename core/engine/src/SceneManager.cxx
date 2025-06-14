#include "playground/SceneManager.hxx"
#include "playground/AssetManager.hxx"
#include <rendering/Rendering.hxx>
#include <rendering/RenderFrame.hxx>
#include <assetloader/AssetLoader.hxx>
#include <algorithm>

namespace playground::scenemanager {

    uint64_t _currentTick = 0;

    void Init() {
    }

    void Update() {
        auto frame = rendering::RenderFrame();
        /*
        for(auto& go : _gameObjects) {
            if (go == nullptr) {
                continue;
            }

            assetmanager::ModelHandle* modelToDraw = 0;
            assetmanager::MaterialHandle* materialToDraw = 0;

            if (go->meshComponent != nullptr) {
                modelToDraw = reinterpret_cast<assetmanager::ModelHandle*>(go->meshComponent->model);
                if (modelToDraw == nullptr || modelToDraw->state != assetmanager::ResourceState::Uploaded) {
                    continue;
                }

                materialToDraw = reinterpret_cast<assetmanager::MaterialHandle*>(go->meshComponent->material);
                if (materialToDraw == nullptr || materialToDraw->state != assetmanager::ResourceState::Uploaded) {
                    continue;
                }

                auto mesh = modelToDraw->meshes[go->meshComponent->meshId];
                auto material = materialToDraw->material;

                rendering::DrawCall::InstanceData newInstance{
                    .position = go->transform.position,
                    .rotation = go->transform.rotation,
                    .scale = go->transform.scale
                };

                auto existingIt = std::find_if(frame.drawCalls.begin(), frame.drawCalls.end(),
                    [&](const rendering::DrawCall& dc) {
                        return dc.vertexBuffer == mesh.vertexBuffer &&
                            dc.indexBuffer == mesh.indexBuffer &&
                            dc.material == material;
                    });

                if (existingIt != frame.drawCalls.end()) {
                    // Found existing draw call - append instance data and increment count
                    existingIt->instanceData.push_back(newInstance);
                    existingIt->instances = static_cast<uint16_t>(existingIt->instanceData.size());
                }
                else {
                    // Create a new draw call entry
                    rendering::DrawCall newDrawCall{
                        .instances = 1,
                        .vertexBuffer = mesh.vertexBuffer,
                        .indexBuffer = mesh.indexBuffer,
                        .material = material,
                        .instanceData = { newInstance }
                    };
                    frame.drawCalls.push_back(std::move(newDrawCall));
                }
            }
        }

        */
        rendering::SubmitFrame(frame);

        _currentTick++;
    }
}
