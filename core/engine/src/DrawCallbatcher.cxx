#include "playground/DrawCallBatcher.hxx"
#include "playground/AssetManager.hxx"
#include "playground/Constants.hxx"
#include <rendering/RenderFrame.hxx>
#include <rendering/Rendering.hxx>
#include <rendering/Constants.hxx>
#include <rendering/DirectionalLight.hxx>
#include <rendering/Camera.hxx>
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

    Allocator alloc(&arena, "Batcher Allocator");

    moodycamel::ConcurrentQueue<DrawCallRange> batches;
    std::atomic<uint32_t> batchIndex = 0;
    std::mutex mutex;
    rendering::DirectionalLight sun;
    eastl::vector<rendering::Camera, Allocator> cameras(alloc);

    void Batch(drawcallbatcher::DrawCall* batch, uint16_t count) {
        ZoneScopedN("Batcher: Batch");
        uint64_t hash = 0;
        batches.enqueue(DrawCallRange{ .start = batch, .count = count });
    }

    void SetSun(math::Vector3 direction, math::Vector4 colour, float intensity) {
        sun = rendering::DirectionalLight{
            .direction = math::Vector4(math::Normalize(direction), 0.0f),
            .colour = colour,
            .intensity = intensity
        };
    }

    void AddCamera(uint8_t order, float fov, float nearPlane, float farPlane, math::Vector3& position, math::Quaternion& rotation) {
        ZoneScopedN("Batcher: Add Camera");

        rendering::Camera camera(fov, 0, nearPlane, farPlane, position, rotation, order);

        cameras.push_back(camera);
    }

    void Submit() {
        ZoneScopedN("Batcher: Submit");
        rendering::RenderFrame frame;

        for (auto& cam : cameras) {
            frame.cameras.emplace_back(std::move(cam));
        }

        frame.sun = sun;
        eastl::hash_map<BatchKey, uint64_t, eastl::hash<BatchKey>, eastl::equal_to<BatchKey>, Allocator, false> batchedDrawCalls(alloc);

        frame.isDirty = true;

        DrawCallRange next;
        while(batches.try_dequeue(next)) {
            ZoneScopedN("Batcher: Process Batch");
            for (int y = 0; y < next.count; y++) {
                auto item = next.start[y];

                if (item.modelHandle == nullptr || item.materialHandle == nullptr) {
                    continue;
                }

                auto modelState = item.modelHandle->state.load();
                auto materialState = item.materialHandle->state.load();

                if (modelState != assetmanager::ResourceState::Uploaded || materialState != assetmanager::ResourceState::Uploaded) {
                    continue;
                }

                const auto& mesh = item.modelHandle->meshes[item.meshId];
                BatchKey key{ item.materialHandle->material, mesh.vertexBuffer, mesh.indexBuffer };

                auto it = batchedDrawCalls.find(key);
                if (it != batchedDrawCalls.end() && frame.drawCalls[it->second].instanceData.size() < rendering::MAX_BATCH_SIZE) {
                    auto& existingDrawCall = frame.drawCalls[it->second];

                    math::Matrix3x3 normalMatrixInput = item.transform.ToMatrix3x3();
                    math::Matrix3x3 normalMatrixInv;
                    math::Inverse(normalMatrixInput, &normalMatrixInv);
                    math::Matrix3x3 normalMatrixInvTranspose;
                    math::Transpose(normalMatrixInv, &normalMatrixInvTranspose);

                    existingDrawCall.instanceData.push_back({
                        .transform = item.transform,
                        .normals = normalMatrixInvTranspose.ToMatrix4x4()
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

                    math::Matrix3x3 normalMatrixInput = item.transform.ToMatrix3x3();
                    math::Matrix3x3 normalMatrixInv;
                    math::Inverse(normalMatrixInput, &normalMatrixInv);
                    math::Matrix3x3 normalMatrixInvTranspose;
                    math::Transpose(normalMatrixInv, &normalMatrixInvTranspose);

                    newDrawCall.instanceData.push_back({ .transform = item.transform, .normals = normalMatrixInvTranspose.ToMatrix4x4() });

                    frame.drawCalls.push_back(newDrawCall);

                    batchedDrawCalls.insert_or_assign(key, newIndex);
                }
            }
        }

        {
            ZoneScopedN("Batcher: Queue Draw Calls");

            batchedDrawCalls.clear();

            cameras.clear();

            rendering::SubmitFrame(std::move(frame));

            arena.Reset();
        }
    }
}
