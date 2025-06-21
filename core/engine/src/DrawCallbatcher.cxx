#include "playground/DrawCallBatcher.hxx"
#include "playground/AssetManager.hxx"
#include "playground/Constants.hxx"
#include <rendering/RenderFrame.hxx>
#include <rendering/Rendering.hxx>
#include <rendering/Constants.hxx>
#include <rendering/DirectionalLight.hxx>
#include <array>
#include <atomic>
#include <mutex>
#include <unordered_map>
#include <EASTL/hash_map.h>
#include <EASTL/queue.h>
#include <EASTL/vector.h>
#include <EASTL/functional.h>
#include <tracy/Tracy.hpp>
#include <shared/Memory.hxx>
#include <math/Math.hxx>
#include <shared/Arena.hxx>
#include <concurrentqueue.h>
#include <iostream>

struct BatchKey {
    playground::rendering::MaterialHandle material;
    playground::rendering::VertexBufferHandle vertexBuffer;
    playground::rendering::IndexBufferHandle indexBuffer;

    bool operator==(const BatchKey& other) const {
        return material == other.material &&
            vertexBuffer == other.vertexBuffer &&
            indexBuffer == other.indexBuffer;
    }
};

// Hash function for BatchKey (specialize eastl::hash)
namespace eastl {
    template<>
    struct hash<BatchKey> {
        size_t operator()(const BatchKey& key) const {
            size_t h1 = eastl::hash<uint32_t>{}(static_cast<uint32_t>(key.material));
            size_t h2 = eastl::hash<uint32_t>{}(static_cast<uint32_t>(key.vertexBuffer));
            size_t h3 = eastl::hash<uint32_t>{}(static_cast<uint32_t>(key.indexBuffer));
            return h1 ^ (h2 << 1) ^ (h3 << 2);
        }
    };
}

namespace playground::drawcallbatcher {
    using ArenaType = memory::VirtualArena;
    using Allocator = memory::ArenaAllocator<ArenaType>;
    ArenaType arena(2048);

    // Swift passes the draw calls as a pointer memory range + size.
    // Since we allocate a large poitner in Swift at boot for performance reason
    // and only update the values on the next frame and never deallocate until shutdown,
    // the ranegs stay valid until the next frame. Thus for efficiency reasons we stroe the ranges and never copy
    struct DrawCallRange {
        DrawCall* start = nullptr;
        size_t count = 0;
    };

    Allocator alloc(&arena);

    eastl::array<DrawCallRange, MAX_ECS_WORKER_THREAD_COUNT> batches;
    std::atomic<uint32_t> batchIndex = 0;
    std::mutex mutex;
    rendering::DirectionalLight sun;

    void Batch(drawcallbatcher::DrawCall* batch, uint16_t count) {
        ZoneScopedN("Batcher: Batch");
        auto index = batchIndex.fetch_add(1, std::memory_order_relaxed);
        uint64_t hash = 0;

        batches[index] = DrawCallRange{ .start = batch, .count = count };
    }

    void SetSun(glm::vec3 direction, glm::vec4 colour, float intensity) {
        sun = rendering::DirectionalLight{
            .direction = glm::vec4(glm::normalize(direction), 0),
            .colour = colour,
            .intensity = intensity
        };
    }

    void Submit() {
        ZoneScopedN("Batcher: Submit");
        rendering::RenderFrame frame;

        frame.sun = sun;

        Allocator alloc(&arena, "Batch Allocator");

        eastl::hash_map<BatchKey, uint64_t, eastl::hash<BatchKey>, eastl::equal_to<BatchKey>, Allocator, false> batchedDrawCalls(alloc);

        frame.isDirty = true;

        for (int x = 0; x < MAX_ECS_WORKER_THREAD_COUNT; x++) {
            ZoneScopedN("Batcher: Process Batch");
            DrawCallRange& next = batches[x];

            for (int y = 0; y < next.count; y++) {
                auto item = next.start[y];

                if (!item.modelHandle || !item.materialHandle) continue;
                if (item.modelHandle->state != assetmanager::ResourceState::Uploaded ||
                    item.materialHandle->state != assetmanager::ResourceState::Uploaded)
                    continue;

                const auto& mesh = item.modelHandle->meshes[item.meshId];
                BatchKey key{ item.materialHandle->material, mesh.vertexBuffer, mesh.indexBuffer };

                auto it = batchedDrawCalls.find(key);
                if (it != batchedDrawCalls.end() && frame.drawCalls[it->second].instanceData.size() < rendering::MAX_BATCH_SIZE) {
                    auto& existingDrawCall = frame.drawCalls[it->second];
                    glm::mat4 modelInv = glm::inverse(item.transform);
                    modelInv = glm::transpose(modelInv);
                    existingDrawCall.instanceData.push_back({
                        .transform = item.transform,
                        .normals = modelInv
                    });
                }
                else {
                    // Create new batch entry
                    size_t newIndex = frame.drawCalls.size();
                    rendering::DrawCall newDrawCall;
                    newDrawCall.vertexBuffer = mesh.vertexBuffer;
                    newDrawCall.indexBuffer = mesh.indexBuffer;
                    newDrawCall.material = item.materialHandle->material;
                    newDrawCall.instanceData.reserve(rendering::MAX_BATCH_SIZE);

                    glm::mat4 modelInv = glm::inverse(item.transform);
                    modelInv = glm::transpose(modelInv);
                    newDrawCall.instanceData.push_back({ .transform = item.transform, .normals = modelInv });

                    frame.drawCalls.push_back(newDrawCall);

                    batchedDrawCalls.insert_or_assign(key, newIndex);
                }
            }
        }

        {
            ZoneScopedN("Batcher: Queue Draw Calls");

            batchIndex.store(0, std::memory_order_relaxed);

            batchedDrawCalls.clear();

            rendering::SubmitFrame(std::move(frame));

            arena.Reset();
        }
    }
}
