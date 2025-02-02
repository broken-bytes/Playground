#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <rendering/IndexBufferHandle.hxx>
#include <rendering/MaterialHandle.hxx>
#include <rendering/VertexBufferHandle.hxx>

namespace playground::rendering {
	// Lifecycle
	auto Init(void* window, uint32_t width, uint32_t height) -> void;
	auto Shutdown() -> void;
    auto PreFrame() -> void;
	auto Update(float deltaTime) -> void;
	auto PostFrame() -> void;

	// Resource creation
	auto CreateVertexBuffer(const void* data, size_t size) -> VertexBufferHandle;
	auto CreateIndexBuffer(const uint32_t* indices, size_t len) -> IndexBufferHandle;
	auto UpdateVertexBuffer(VertexBufferHandle buffer, const void* data, size_t size) -> void;
	auto UpdateIndexBuffer(IndexBufferHandle buffer, const void* data, size_t size) -> void;

	// Resource destruction
	auto UnloadShader(uint64_t shader) -> void;

	// State management
	auto SetClearColor(float r, float g, float b, float a) -> void;
	auto SetViewport(int x, int y, int width, int height) -> void;
	auto DrawIndexed(VertexBufferHandle vertexBuffer, IndexBufferHandle indexBuffer, MaterialHandle material) -> void;
}
