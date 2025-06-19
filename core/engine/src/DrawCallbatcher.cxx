#include "playground/DrawCallBatcher.hxx"
#include "playground/AssetManager.hxx"
#include <rendering/RenderFrame.hxx>
#include <rendering/Rendering.hxx>
#include <mutex>
#include <unordered_map>

namespace playground::drawcallbatcher {
    struct DrawCallKey {
        rendering::MaterialHandle material;
        rendering::VertexBufferHandle vertexBuffer;
        rendering::IndexBufferHandle indexBuffer;

        bool operator==(const DrawCallKey& other) const {
            return material == other.material &&
                vertexBuffer == other.vertexBuffer &&
                indexBuffer == other.indexBuffer;
        }
    };

    size_t HashDrawCallKey(const DrawCallKey& key) {
        return key.material ^ (key.vertexBuffer << 1) ^ (key.indexBuffer << 2);
    }

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

        std::unordered_map<uint64_t, rendering::DrawCall> batchedDrawCalls;

        for (const auto& dc : drawCalls) {
            if (dc.modelHandle != nullptr &&
                dc.materialHandle != nullptr &&
                dc.modelHandle->state == assetmanager::ResourceState::Uploaded &&
                dc.materialHandle->state == assetmanager::ResourceState::Uploaded) {

                DrawCallKey key{
                    .material = dc.materialHandle->material,
                    .vertexBuffer = dc.modelHandle->meshes[dc.meshId].vertexBuffer,
                    .indexBuffer = dc.modelHandle->meshes[dc.meshId].indexBuffer
                };

                auto hash = HashDrawCallKey(key);

                auto it = batchedDrawCalls.find(hash);
                if (it != batchedDrawCalls.end()) {
                    it->second.instances += 1;
                    it->second.instanceData.emplace_back(dc.transform);
                }
                else {
                    auto instanceData = std::vector<rendering::DrawCall::InstanceData>();
                    instanceData.reserve(32);
                    instanceData.emplace_back(dc.transform);
                    rendering::DrawCall newDrawCall{
                        .instances = 1,
                        .vertexBuffer = dc.modelHandle->meshes[dc.meshId].vertexBuffer,
                        .indexBuffer = dc.modelHandle->meshes[dc.meshId].indexBuffer,
                        .material = dc.materialHandle->material,
                        .instanceData = { rendering::DrawCall::InstanceData {.transform = dc.transform } }
                    };
                    batchedDrawCalls.emplace(hash, std::move(newDrawCall));
                }
            }
        }

        frame.drawCalls.reserve(batchedDrawCalls.size());
        for (auto& [key, drawCall] : batchedDrawCalls) {
            frame.drawCalls.push_back(std::move(drawCall));
        }

        drawCalls.clear();

        rendering::SubmitFrame(frame);
    }
}
