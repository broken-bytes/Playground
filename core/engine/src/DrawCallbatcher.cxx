#include "playground/DrawCallBatcher.hxx"
#include "playground/AssetManager.hxx"
#include <rendering/RenderFrame.hxx>
#include <rendering/Rendering.hxx>
#include <mutex>

namespace playground::drawcallbatcher {
    std::vector<drawcallbatcher::DrawCall> drawCalls = {};
    std::mutex mtx;

    void Batch(drawcallbatcher::DrawCall* batch, uint16_t count) {
        std::lock_guard<std::mutex> lock(mtx);

        for (int x = 0; x < count; x++) {
            drawCalls.push_back(batch[x]);
        }
    }

    void Submit() {
        rendering::RenderFrame frame;
        frame.isDirty = true;

        for (const auto& dc : drawCalls) {
            if (
                dc.modelHandle != nullptr &&
                dc.materialHandle != nullptr &&
                dc.modelHandle->state == assetmanager::ResourceState::Uploaded &&
                dc.materialHandle->state == assetmanager::ResourceState::Uploaded
            ) {
                // Try to find an existing entry with the same material, vertexBuffer, and indexBuffer
                auto it = std::find_if(frame.drawCalls.begin(), frame.drawCalls.end(),
                    [&](const playground::rendering::DrawCall& existing) {
                        return existing.material == dc.materialHandle->material &&
                            existing.vertexBuffer == dc.modelHandle->meshes[dc.meshId].vertexBuffer &&
                            existing.indexBuffer == dc.modelHandle->meshes[dc.meshId].indexBuffer;
                    });

                if (it != frame.drawCalls.end()) {
                    it->instances += 1;
                    it->instanceData.push_back(rendering::DrawCall::InstanceData { .transform = dc.transform });
                }
                else {
                    rendering::DrawCall drawCall {
                        .instances = 1,
                        .vertexBuffer = dc.modelHandle->meshes[dc.meshId].vertexBuffer,
                        .indexBuffer = dc.modelHandle->meshes[dc.meshId].indexBuffer,
                        .material = dc.materialHandle->material,
                        .instanceData = { rendering::DrawCall::InstanceData { .transform = dc.transform } }
                    };
                    frame.drawCalls.push_back(drawCall);
                }
            }
        }

        drawCalls.clear();

        rendering::SubmitFrame(frame);
    }
}
