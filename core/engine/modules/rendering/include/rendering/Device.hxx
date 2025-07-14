#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include "rendering/CommandList.hxx"
#include "rendering/CommandListType.hxx"
#include "rendering/CommandQueue.hxx"
#include "rendering/ConstantBuffer.hxx"
#include "rendering/StructuredBuffer.hxx"
#include "rendering/Context.hxx"
#include "rendering/GraphicsContext.hxx"
#include "rendering/UploadContext.hxx"
#include "rendering/Fence.hxx"
#include "rendering/Heap.hxx"
#include "rendering/TextureFormat.hxx"
#include "rendering/Shader.hxx"
#include "rendering/ShadowMap.hxx"
#include "rendering/Swapchain.hxx"
#include "rendering/IndexBuffer.hxx"
#include "rendering/VertexBuffer.hxx"
#include "rendering/InstanceBuffer.hxx"
#include "rendering/DepthBuffer.hxx"
#include "rendering/Sampler.hxx"
#include "rendering/Cubemap.hxx"

#include <shared/Arena.hxx>

namespace playground::rendering {
	class Buffer;
	class RenderTarget;

    using ArenaType = memory::VirtualArena;
    using Allocator = memory::ArenaAllocator<ArenaType>;

	class Device {

	public:
        Device(uint8_t frameCount) {
            _frameCount = frameCount;
        }
		virtual ~Device() = default;
		virtual auto Flush() -> void = 0;

		// Creation work submission and synchronization
		virtual auto CreateGraphicsContext(std::string name, void* window, uint32_t width, uint32_t height, bool offscreen) -> std::shared_ptr<GraphicsContext> = 0;
		virtual auto CreateUploadContext(std::string name) -> std::shared_ptr<UploadContext> = 0;
		virtual auto CreateCommandList(
            CommandListType type,
            std::string name = ""
        ) -> std::shared_ptr<CommandList> = 0;
		// Creation of resources
		virtual auto CreateBuffer(uint64_t size) -> std::shared_ptr<Buffer> = 0;
		virtual auto CreateRenderTarget(
            uint32_t width,
            uint32_t height,
            TextureFormat format,
            std::string name = "",
            bool isCPUReadable = false
        ) -> std::shared_ptr<RenderTarget> = 0;
		virtual auto CreateDepthBuffer(
            uint32_t width,
            uint32_t height,
            std::string name = ""
        ) -> std::shared_ptr<DepthBuffer> = 0;
		virtual auto CreateMaterial(std::string& vertexShader, std::string& pixelShader) -> std::shared_ptr<Material> = 0;
		virtual auto CreateVertexBuffer(const void* data, uint64_t size, uint64_t stride, bool isStatic) -> std::shared_ptr<VertexBuffer> = 0;
		virtual auto UpdateVertexBuffer(std::shared_ptr<VertexBuffer> buffer, const void* data, uint64_t size) -> void = 0;
		virtual auto CreateIndexBuffer(const uint32_t* indices, size_t size) -> std::shared_ptr<IndexBuffer> = 0;
		virtual auto UpdateIndexBuffer(std::shared_ptr<IndexBuffer> buffer, std::vector<uint32_t> indices) -> void = 0;
        virtual auto CreateConstantBuffer(const void* data, size_t size, size_t itemSize, ConstantBuffer::BindingMode mode, std::string name) -> std::shared_ptr<ConstantBuffer> = 0;
        virtual auto CreateStructuredBuffer(void* data, size_t size, size_t itemSize, std::string name) -> std::shared_ptr<StructuredBuffer> = 0;
        virtual auto CreateInstanceBuffer(uint64_t count, uint64_t stride) -> std::shared_ptr<InstanceBuffer> = 0;
        virtual auto CreateTexture(uint32_t width, uint32_t height, std::vector<std::vector<uint8_t>> mips, Allocator& allocator) -> std::shared_ptr<Texture> = 0;
        virtual auto CreateCubemap(uint32_t width, uint32_t height, std::vector<std::vector<std::vector<uint8_t>>> faces, Allocator& allocator) -> std::shared_ptr<Cubemap> = 0;
        virtual auto CreateSampler(TextureFiltering filtering, TextureWrapping wrapping) -> std::shared_ptr<Sampler> = 0;
        virtual auto CreateShadowMap(uint32_t width, uint32_t height, std::string name) -> std::shared_ptr<ShadowMap> = 0;
        virtual auto CreateSwapchain(uint8_t bufferCount, uint16_t width, uint16_t height, void* window) -> std::shared_ptr<Swapchain> = 0;
        virtual auto CreateShadowMaterial(const std::string vertexShader) -> std::shared_ptr<Material> = 0;

        virtual auto GetSrvHeap() -> std::shared_ptr<Heap> = 0;
        virtual auto GetSamplerHeap() -> std::shared_ptr<Heap> = 0;

		// Deleting resources
		virtual auto DestroyShader(uint64_t shaderHandle) -> void = 0;
        virtual auto WaitForIdleGPU() -> void = 0;

    protected:
        uint8_t _frameCount;
	};
}
