#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#define GLM_FORCE_CTOR_INIT
#undef near
#undef far
#include <cstdint>
#include <string>
#include <vector>
#include <rendering/IndexBufferHandle.hxx>
#include <rendering/MaterialHandle.hxx>
#include <rendering/VertexBufferHandle.hxx>
#include <rendering/TextureHandle.hxx>
#include <rendering/CameraHandle.hxx>
#include <assetloader/RawMeshData.hxx>
#include <assetloader/RawTextureData.hxx>

namespace playground::rendering {
	// Lifecycle
	auto Init(void* window, uint32_t width, uint32_t height, bool offscreen) -> void;
	auto Shutdown() -> void;
    auto PreFrame() -> void;
	auto Update(double deltaTime) -> void;
	auto PostFrame() -> void;

    /// Readback the current frame buffer
    /// When data is nullptr, the function will return the size of the buffer required to readback the frame buffer
    auto ReadbackBuffer(void* data) -> size_t;

	// Resource creation
	auto CreateVertexBuffer(const void* data, size_t size, size_t stride, bool isStatic) -> VertexBufferHandle;
	auto CreateIndexBuffer(const uint32_t* indices, size_t len) -> IndexBufferHandle;
	auto UpdateVertexBuffer(VertexBufferHandle buffer, const void* data, size_t size) -> void;
	auto UpdateIndexBuffer(IndexBufferHandle buffer, const void* data, size_t size) -> void;

    auto UploadMesh(const assetloader::RawMeshData& mesh) -> std::pair<VertexBufferHandle, IndexBufferHandle>;
    auto UploadTexture(const assetloader::RawTextureData& texture) -> TextureHandle;

	// Resource destruction
	auto UnloadShader(uint64_t shader) -> void;
	auto DrawIndexed(VertexBufferHandle vertexBuffer, IndexBufferHandle indexBuffer, MaterialHandle material) -> void;


    auto CreateCamera(
        float fov,
        float aspectRatio,
        float near,
        float far,
        float pos[3],
        float rot[3],
        uint32_t renderTargetTextureId
    ) -> CameraHandle;

    auto SetCameraFOV(
        CameraHandle handle,
        float fov
    ) -> void;

    auto SetCameraAspectRatio(
        CameraHandle handle,
        float aspectRatio
    ) -> void;

    auto SetCameraNear(
        CameraHandle handle,
        float near
    ) -> void;

    auto SetCameraFar(
        CameraHandle handle,
        float far
    ) -> void;

    auto SetCameraPosition(
        CameraHandle handle,
        float pos[3]
    ) -> void;

    auto SetCameraRotation(
        CameraHandle handle,
        float rot[4]
    ) -> void;

    auto SetCameraRenderTarget(
        CameraHandle handle,
        uint32_t texture
    ) -> void;
}
