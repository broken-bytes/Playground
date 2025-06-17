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
#include <vector>
#include <rendering/IndexBufferHandle.hxx>
#include <rendering/MaterialHandle.hxx>
#include <rendering/Mesh.hxx>
#include <rendering/VertexBufferHandle.hxx>
#include <rendering/TextureHandle.hxx>
#include <rendering/CameraHandle.hxx>
#include "rendering/ModelUploadJob.hxx"
#include "rendering/MaterialUploadJob.hxx"
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

    auto QueueUploadModel(std::vector<assetloader::RawMeshData> meshes, uint32_t, std::function<void(uint32_t, std::vector<Mesh>)>) -> void;
    auto QueueUploadMaterial(std::string vertexShaderCode, std::string pixelShaderCode, uint32_t handle, std::function<void(uint32_t, uint32_t)>) -> void;

    auto UploadModel(ModelUploadJob job) -> void;
    auto UploadTexture(const assetloader::RawTextureData& texture) -> TextureHandle;
    auto CreateMaterial(MaterialUploadJob job) -> void;

    auto SubmitFrame(RenderFrame frame) -> void;
}
