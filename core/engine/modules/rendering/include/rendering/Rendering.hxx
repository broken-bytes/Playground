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
#include <rendering/CameraHandle.hxx>

namespace playground::rendering {
	// Lifecycle
	auto Init(void* window, uint32_t width, uint32_t height) -> void;
	auto Shutdown() -> void;
    auto PreFrame() -> void;
	auto Update(double deltaTime) -> void;
	auto PostFrame() -> void;

	// Resource creation
	auto CreateVertexBuffer(const void* data, size_t size, size_t stride) -> VertexBufferHandle;
	auto CreateIndexBuffer(const uint32_t* indices, size_t len) -> IndexBufferHandle;
	auto UpdateVertexBuffer(VertexBufferHandle buffer, const void* data, size_t size) -> void;
	auto UpdateIndexBuffer(IndexBufferHandle buffer, const void* data, size_t size) -> void;

	// Resource destruction
	auto UnloadShader(uint64_t shader) -> void;
	auto DrawIndexed(VertexBufferHandle vertexBuffer, IndexBufferHandle indexBuffer, MaterialHandle material) -> void;

    auto SetCamera(
        CameraHandle handle,
        float fov,
        float aspectRatio,
        float near,
        float far,
        float pos[3],
        float rot[3]
    ) -> void;
}
