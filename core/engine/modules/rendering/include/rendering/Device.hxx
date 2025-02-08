#pragma once

#include <cstdint>
#include <memory>
#include "rendering/CommandList.hxx"
#include "rendering/CommandListType.hxx"
#include "rendering/CommandQueue.hxx"
#include "rendering/ConstantBuffer.hxx"
#include "rendering/Context.hxx"
#include "rendering/GraphicsContext.hxx"
#include "rendering/UploadContext.hxx"
#include "rendering/Fence.hxx"
#include "rendering/Heap.hxx"
#include "rendering/TextureFormat.hxx"
#include "rendering/Shader.hxx"
#include "rendering/Swapchain.hxx"
#include "rendering/IndexBuffer.hxx"
#include "rendering/VertexBuffer.hxx"
#include "rendering/DepthBuffer.hxx"
#include "rendering/RootSignature.hxx"
#include "rendering/Rendering.hxx"
#include "rendering/PipelineState.hxx"
#include "rendering/Sampler.hxx"

namespace playground::rendering {
	class Buffer;
	class RenderTarget;

	class Device {

	public:
        Device(uint8_t frameCount) {
            _frameCount = frameCount;
        }
		virtual ~Device() = default;
		virtual auto Flush() -> void = 0;

		// Creation work submission and synchronization
		virtual auto CreateGraphicsContext(void* window, uint32_t width, uint32_t height, uint8_t bufferCount, bool offscreen) -> std::unique_ptr<GraphicsContext> = 0;
		virtual auto CreateUploadContext() -> std::unique_ptr<UploadContext> = 0;
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
		virtual auto CreateMaterial(
			std::map<ShaderType, std::shared_ptr<Shader>> shaders,
			std::map<std::string, uint64_t> textures = {},
			std::map<std::string, float> floats = {},
			std::map<std::string, uint32_t> ints = {},
			std::map<std::string, bool> bools = {},
			std::map<std::string, std::array<float, 2>> vec2s = {},
			std::map<std::string, std::array<float, 3>> vec3s = {},
			std::map<std::string, std::array<float, 4>> vec4s = {}
		) -> std::shared_ptr<Material> = 0;
		virtual auto CreatePipelineState(
            const std::string& vertexShader,
            const std::string& pixelShader
		) -> std::shared_ptr<PipelineState> = 0;
        virtual auto GetRootSignature() -> std::shared_ptr<RootSignature> = 0;
		virtual auto CreateVertexBuffer(const void* data, uint64_t size, uint64_t stride, bool isStatic) -> std::shared_ptr<VertexBuffer> = 0;
		virtual auto UpdateVertexBuffer(std::shared_ptr<VertexBuffer> buffer, const void* data, uint64_t size) -> void = 0;
		virtual auto CreateIndexBuffer(const uint32_t* indices, size_t size) -> std::shared_ptr<IndexBuffer> = 0;
		virtual auto UpdateIndexBuffer(std::shared_ptr<IndexBuffer> buffer, std::vector<uint32_t> indices) -> void = 0;
        virtual auto CreateConstantBuffer(void* data, size_t size, std::string name) -> std::shared_ptr<ConstantBuffer> = 0;
        virtual auto CreateTexture(uint32_t width, uint32_t height, const uint8_t* data) -> std::shared_ptr<Texture> = 0;
        virtual auto CreateSampler(TextureFiltering filtering, TextureWrapping wrapping) -> std::shared_ptr<Sampler> = 0;
		// Deleting resources
		virtual auto DestroyShader(uint64_t shaderHandle) -> void = 0;

    protected:
        uint8_t _frameCount;
	};
}
