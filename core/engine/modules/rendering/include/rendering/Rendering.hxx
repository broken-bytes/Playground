#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#define GLM_FORCE_CTOR_INIT
#undef near
#undef far
#include <cstdint>
#include <functional>
#include <semaphore>
#include <string>
#include <map>
#include <memory>
#include <vector>
#include <rendering/IndexBufferHandle.hxx>
#include <rendering/MaterialHandle.hxx>
#include <rendering/Mesh.hxx>
#include <rendering/VertexBufferHandle.hxx>
#include <rendering/TextureHandle.hxx>
#include <rendering/CameraHandle.hxx>
#include "rendering/ModelUploadJob.hxx"
#include "rendering/MaterialUploadJob.hxx"
#include "rendering/TextureUploadJob.hxx"
#include "rendering/CubemapUploadJob.hxx"
#include "rendering/RenderFrame.hxx"
#include <assetloader/RawMeshData.hxx>
#include <assetloader/RawTextureData.hxx>
#include <assetloader/RawMaterialData.hxx>
#include <assetloader/RawShaderData.hxx>
#include <tuple>
#include <future>

namespace playground::rendering {
	// Lifecycle
	auto Init(
        void* window,
        uint32_t width,
        uint32_t height,
        bool offscreen,
        std::promise<void>& rendererReadyPromise
    ) -> void;
	auto Shutdown() -> void;
    auto PreFrame() -> void;
	auto Update() -> void;
	auto PostFrame() -> void;

    /// Readback the current frame buffer
    /// When data is nullptr, the function will return the size of the buffer required to readback the frame buffer
    auto ReadbackBuffer(void* data) -> size_t;

	// Resource creation
	auto CreateVertexBuffer(const void* data, size_t size, size_t stride, bool isStatic) -> VertexBufferHandle;
	auto CreateIndexBuffer(const uint32_t* indices, size_t len) -> IndexBufferHandle;
	auto UpdateVertexBuffer(VertexBufferHandle buffer, const void* data, size_t size) -> void;
	auto UpdateIndexBuffer(IndexBufferHandle buffer, const void* data, size_t size) -> void;

    auto QueueUploadModel(std::vector<assetloader::RawMeshData>& meshes, uint32_t, std::function<void(uint32_t, std::vector<Mesh>)>) -> void;
    auto QueueUploadMaterial(
        std::string vertexShaderCode,
        std::string pixelShaderCode,
        MaterialType type,
        uint32_t handle,
        std::function<void(uint32_t, uint32_t)>,
        void (*onCompletion)(uint32_t) = nullptr
    ) -> void;
    auto QueueUploadTexture(assetloader::RawTextureData* texture, uint32_t handle, void (*callback)(uint32_t, uint32_t)) -> void;
    auto QueueUploadCubemap(std::shared_ptr<assetloader::RawCubemapData> cubemap, uint32_t handle, std::function<void(uint32_t, uint32_t)> callback) -> void;
    auto UploadModel(ModelUploadJob& job) -> void;
    auto UploadTexture(TextureUploadJob* job) -> void;
    auto UploadCubemap(CubemapUploadJob& job) -> void;
    auto CreateMaterial(MaterialUploadJob job) -> void;
    auto RegisterShadowShader(
        std::string& vertexShaderCode
    ) -> void;

    void RegisterPostProcessingMaterial(uint8_t slot, uint32_t materialId);
    void UnregisterPostProcessingMaterial(uint8_t slot);

    auto SetSkyboxMaterial(uint32_t materialId) -> void;

    // Material setup
    void SetMaterialTexture(uint32_t materialId, uint8_t slot, uint32_t textureId);
    void SetMaterialCubemap(uint32_t materialId, uint8_t slot, uint32_t cubemapId);
    void SetMaterialFloat(uint32_t materialId, uint8_t slot, float value);

    auto SubmitFrame(RenderFrame frame) -> void;
}
