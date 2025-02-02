#include "rendering/Rendering.hxx"
#include "rendering/Device.hxx"
#include "rendering/Frame.hxx"
#include "rendering/VertexBuffer.hxx"
#include "rendering/DeviceFactory.hxx"
#include "rendering/Texture.hxx"
#include "rendering/renderpasses/OpaqueRenderPass.hxx"
#include "rendering/CommandQueue.hxx"
#include "rendering/GraphicsContext.hxx"
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

	std::unique_ptr<GraphicsContext> graphicsContext = nullptr;

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
            std::stringstream ss;
            ss << "Frame " << x;

			auto rendertarget = device->CreateRenderTarget(width, height, TextureFormat::RGBA8, ss.str());

            ss.clear();
            ss << "DepthBuffer " << x;

			auto depthBuffer = device->CreateDepthBuffer(width, height, ss.str());

			frames.emplace_back(std::make_shared<Frame>(rendertarget, depthBuffer));
		}

		// Create all command lists
		opaqueCommandList = device->CreateCommandList(CommandListType::Graphics, "OpaqueCommandList");

		transparentCommandList = device->CreateCommandList(CommandListType::Graphics, "TransparentCommandList");

		shadowCommandList = device->CreateCommandList(CommandListType::Graphics, "ShadowCommandList");

		uiCommandList = device->CreateCommandList(CommandListType::Graphics, "UICommandList");

        transferCommandList = device->CreateCommandList(CommandListType::Transfer, "TransferCommandList");

		graphicsContext = device->CreateGraphicsContext(window, width, height, FRAME_COUNT);

		// Create the render passes
		opaqueRenderPass = std::make_unique<OpaqueRenderPass>();
	}

	auto Shutdown() -> void {
        // Destroy all render passes first as these hold references to queues, buffers etc.
        opaqueRenderPass = nullptr;

        // Clear the contexts -> These hold swapchains and other resources
        graphicsContext = nullptr;

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
        transferCommandList = nullptr;

        // Destroy all resources (render targets, depth buffers, etc)
        for (auto& frame : frames)
        {
            frame = nullptr;
        }

		// Cleanup
		device = nullptr;

        FreeImage_DeInitialise();
	}

	auto PreFrame() -> void {
		auto renderTarget = frames[frameIndex]->RenderTarget();
		auto depthBuffer = frames[frameIndex]->DepthBuffer();

		opaqueRenderPass->Begin(opaqueCommandList, renderTarget, depthBuffer);
	}

	auto Update(float deltaTime) -> void {
		opaqueRenderPass->Execute();
	}

	auto PostFrame() -> void {
		opaqueRenderPass->End();

        opaqueCommandList->Close();
        transparentCommandList->Close();
        shadowCommandList->Close();
        uiCommandList->Close();
        transferCommandList->Close();

        graphicsContext->ExecuteCommandLists({ opaqueCommandList });
        graphicsContext->CopyToBackBuffer(frames[frameIndex]->RenderTarget());
		graphicsContext->Finish();

        opaqueCommandList->Reset();
        transparentCommandList->Reset();
        shadowCommandList->Reset();
        uiCommandList->Reset();

        transferCommandList->Reset();

        frameIndex = (frameIndex + 1) % FRAME_COUNT;
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
