#include "rendering/Rendering.hxx"
#include "rendering/Device.hxx"
#include "rendering/Frame.hxx"
#include "rendering/VertexBuffer.hxx"
#include "rendering/DeviceFactory.hxx"
#include "rendering/Texture.hxx"
#include "rendering/renderpasses/OpaqueRenderPass.hxx"
#include "rendering/CommandQueue.hxx"

#include <map>
#include <memory>
#include <FreeImagePlus.h>

namespace playground::rendering {
	constexpr uint8_t FRAME_COUNT = 2;

	std::shared_ptr<Device> device = nullptr;
	void* window = nullptr;

	uint8_t frameIndex = 0;
	std::vector<std::shared_ptr<Frame>> frames = {};

	// Resource management
	std::map<uint64_t, std::shared_ptr<VertexBuffer>> vertexBuffers = {};
	std::map<uint64_t, std::shared_ptr<IndexBuffer>> indexBuffers = {};
	std::map<uint32_t, std::shared_ptr<Texture>> textures = {};
	std::map<uint32_t, std::shared_ptr<Shader>> shaders = {};
	std::map<uint32_t, std::shared_ptr<Material>> materials = {};
	std::map<uint32_t, std::shared_ptr<Mesh>> meshes = {};

	std::unique_ptr<Context> graphicsContext = nullptr;

	std::shared_ptr<CommandList> opaqueCommandList = nullptr;
	std::shared_ptr<CommandList> transparentCommandList = nullptr;
	std::shared_ptr<CommandList> shadowCommandList = nullptr;
	std::shared_ptr<CommandList> uiCommandList = nullptr;
	std::shared_ptr<CommandList> transferCommandList = nullptr;

	std::unique_ptr<OpaqueRenderPass> opaqueRenderPass = nullptr;

	auto Init(void* window, uint32_t width, uint32_t height) -> void {
		FreeImage_Initialise();

		// Create a device
		device = DeviceFactory::CreateDevice(RenderBackendType::D3D12, window, FRAME_COUNT);

		// Create frames (frames hold all render things that need to alter between frames)
		for (int x = 0; x < FRAME_COUNT; x++)
		{
			auto rendertarget = device->CreateRenderTarget(width, height, TextureFormat::BGRA8);
			auto depthBuffer = device->CreateDepthBuffer(width, height);

			frames.emplace_back(std::make_shared<Frame>(rendertarget,depthBuffer));
		}

		// Create all command lists
		opaqueCommandList = device->CreateCommandList(CommandListType::Graphics, "OpaqueCommandList");
        opaqueCommandList->Close();

		transparentCommandList = device->CreateCommandList(CommandListType::Graphics, "TransparentCommandList");
        transparentCommandList->Close();

		shadowCommandList = device->CreateCommandList(CommandListType::Graphics, "ShadowCommandList");
        shadowCommandList->Close();

		uiCommandList = device->CreateCommandList(CommandListType::Graphics, "UICommandList");
        uiCommandList->Close();

        transferCommandList = device->CreateCommandList(CommandListType::Transfer, "TransferCommandList");
        transferCommandList->Close();

		graphicsContext = device->CreateGraphicsContext(window, width, height);

		// Create the render passes
		opaqueRenderPass = std::make_unique<OpaqueRenderPass>();
	}

	auto Shutdown() -> void {
        // Close all command lists
        opaqueCommandList->Close();
        transparentCommandList->Close();
        shadowCommandList->Close();
        uiCommandList->Close();
        transferCommandList->Close();

        opaqueCommandList = nullptr;
        transparentCommandList = nullptr;
        shadowCommandList = nullptr;
        uiCommandList = nullptr;
        shadowCommandList = nullptr;
        transparentCommandList = nullptr;

        // Destroy all resources (render targets, depth buffers, etc)
        for (auto& frame : frames)
        {
            frame = nullptr;
        }

        // Flush the device -> This destroys all heaps related to the device
        device->Flush();

        // Clear the contexts -> These hold swapchains and other resources
		graphicsContext = nullptr;
		// Cleanup
		device = nullptr;
	}

	auto PreFrame() -> void {
        opaqueCommandList->Reset();
        transparentCommandList->Reset();
        shadowCommandList->Reset();
        uiCommandList->Reset();

        transferCommandList->Reset();

		auto renderTarget = frames[frameIndex]->RenderTarget();
		auto depthBuffer = frames[frameIndex]->DepthBuffer();

		opaqueRenderPass->Begin(opaqueCommandList, renderTarget, depthBuffer);
	}

	auto Update(float deltaTime) -> void {
		opaqueRenderPass->Execute();
	}

	auto PostFrame() -> void {
		opaqueRenderPass->End();
		graphicsContext->Finish();

        opaqueCommandList->Close();
        transparentCommandList->Close();
        shadowCommandList->Close();
        uiCommandList->Close();
        transferCommandList->Close();
	}

	auto LoadShader(
		const std::string& code,
		ShaderType type
	) -> uint32_t {
		const auto shader = device->CompileShader(code, type);
		shaders[shader->id] = shader;

		return shader->id;
	}

	auto UnloadShader(uint64_t shader) -> void {
		device->DestroyShader(shader);
	}

	auto CreateVertexBuffer(const void* data, size_t size) -> VertexBufferHandle {
		auto buffer = device->CreateVertexBuffer(data, size);
		auto id = buffer->Id();

		vertexBuffers[id] = buffer;

		return id;
	}

	auto CreateIndexBuffer(const uint32_t* indices, size_t len) -> IndexBufferHandle {
		auto buffer = device->CreateIndexBuffer(indices, len);
		auto id = buffer->Id();

		indexBuffers[id] = buffer;

		return id;
	}

	auto UpdateVertexBuffer(VertexBufferHandle buffer, const void* data, size_t size) -> void {

	}

	auto UpdateIndexBuffer(IndexBufferHandle buffer, const void* data, size_t size) -> void {

	}

	auto DrawIndexed(VertexBufferHandle vertexBuffer, IndexBufferHandle indexBuffer, MaterialHandle material) -> void {
	}
}
