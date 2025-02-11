#include <map>
#include <memory>
#include <sstream>
#include "rendering/Rendering.hxx"
#include "rendering/Device.hxx"
#include "rendering/Frame.hxx"
#include "rendering/VertexBuffer.hxx"
#include "rendering/DeviceFactory.hxx"
#include "rendering/Texture.hxx"
#include "rendering/renderpasses/OpaqueRenderPass.hxx"
#include "rendering/CommandQueue.hxx"
#include "rendering/GraphicsContext.hxx"
#include "rendering/RootSignature.hxx"
#include "rendering/Camera.hxx"
#include "rendering/Sampler.hxx"
#include <assetloader/AssetLoader.hxx>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

struct ObjectData {
    glm::mat4 WorldMatrix;
};

namespace playground::rendering {

    struct Config {
        uint32_t Width;
        uint32_t Height;
        bool Offscreen;
    };

    Config config;

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
    std::unique_ptr<UploadContext> uploadContext = nullptr;

	std::shared_ptr<CommandList> opaqueCommandList = nullptr;
	std::shared_ptr<CommandList> transparentCommandList = nullptr;
	std::shared_ptr<CommandList> shadowCommandList = nullptr;
	std::shared_ptr<CommandList> uiCommandList = nullptr;

	std::unique_ptr<OpaqueRenderPass> opaqueRenderPass = nullptr;

    std::shared_ptr<RootSignature> rootSignature = nullptr;

    std::shared_ptr<PipelineState> defaultPipelineState = nullptr;

    std::vector<std::unique_ptr<Camera>> cameras = {};

    std::shared_ptr<ConstantBuffer> cameraBuffer = nullptr;

    std::shared_ptr<ConstantBuffer> objectBuffer = nullptr;

    std::string vertexShaderCode = R"(
        cbuffer CameraBuffer : register(b0) {
            matrix viewMatrix;
            matrix projectionMatrix;
        };

        cbuffer ObjectBuffer : register(b1) {
            matrix worldMatrix;
        };

        struct VSInput {
            float3 position : POSITION;
            float4 color : COLOR;
            float3 normal : NORMAL;
            float2 uv : TEXCOORD;
        };

        struct VSOutput {
            float4 position : SV_Position;
            float4 color : COLOR;
            float3 normal : NORMAL;
            float2 uv : TEXCOORD;
        };

        VSOutput VSMain(VSInput input) {
            VSOutput output;
    
            // Transform object position to world space
            float4 worldPosition = mul(float4(input.position, 1.0f), worldMatrix);

            // Transform world position to view space
            float4 viewPosition = mul(worldPosition, viewMatrix);

            // Transform view position to clip space
            float4 clipPosition = mul(viewPosition, projectionMatrix);

            // Snap the position to the nearest 0.02 grid in clip space
            clipPosition.xy = round(clipPosition.xy * 50.0f) / 50.0f;

            // Set the output position
            output.position = clipPosition;
    
            // Pass through other attributes
            output.color = input.color;
            output.normal = input.normal;
            output.uv = input.uv;

            return output;
        }
        )";

    std::string pixelShaderCode = R"(    
        Texture2D diffuse : register(t0);
        SamplerState mainSampler : register(s0);

        struct PSInput {
            float4 position : SV_Position;
            float4 color    : COLOR;
            float3 normal   : NORMAL;
            float2 uv       : TEXCOORD;
        };

        float4 PSMain(PSInput input) : SV_TARGET {
            return diffuse.Sample(mainSampler, input.uv);
        }
        )";

    ObjectData objectData;

    bool didUpload = false;

    bool textureUploaded = false;
    bool textureCreated = false;

    std::shared_ptr<Texture> targetTexture = nullptr;
    std::shared_ptr<VertexBuffer> vertexBuffer = nullptr;
    std::shared_ptr<IndexBuffer> indexBuffer = nullptr;

    std::shared_ptr<Sampler> sampler = nullptr;

	auto Init(void* window, uint32_t width, uint32_t height, bool offscreen) -> void {
		// Create a device
		device = DeviceFactory::CreateDevice(RenderBackendType::D3D12, FRAME_COUNT);

        config.Width = width;
        config.Height = height;
        config.Offscreen = offscreen;

		// Create frames (frames hold all render things that need to alter between frames)
		for (int x = 0; x < FRAME_COUNT; x++)
		{
            std::stringstream ss;
            ss << "Frame " << x;

			auto rendertarget = device->CreateRenderTarget(width, height, TextureFormat::BGRA8, ss.str(), offscreen);

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

		graphicsContext = device->CreateGraphicsContext(window, width, height, FRAME_COUNT, offscreen);
        uploadContext = device->CreateUploadContext();

        rootSignature = device->GetRootSignature();

        defaultPipelineState = device->CreatePipelineState(vertexShaderCode, pixelShaderCode);

		// Create the render passes
		opaqueRenderPass = std::make_unique<OpaqueRenderPass>(width, height);

        sampler = device->CreateSampler(TextureFiltering::Point, TextureWrapping::Clamp);

        auto camera = Camera(60, width / (float)(height), 0.1f, 100.0f, glm::vec3(0, 0, 0), glm::quat(0, 0, 0, 1), 0);
        cameras.push_back(std::make_unique<Camera>(camera));

        cameraBuffer = device->CreateConstantBuffer(&cameras[0], sizeof(CameraData), "CameraBuffer");

        objectBuffer = device->CreateConstantBuffer(nullptr, sizeof(ObjectData), "ObjectBuffer");

        glm::mat4 scale = glm::identity<glm::mat4>();
        glm::mat4 transform = glm::translate(scale, glm::vec3(0, 0, 5.0f));
        glm::quat quaternion = glm::angleAxis(glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 rotationMatrix = glm::mat4_cast(quaternion);
        glm::mat4 finalTransform = glm::transpose(transform * rotationMatrix);
        objectData.WorldMatrix = finalTransform;
	}

	auto Shutdown() -> void {
        // Destroy all render passes first as these hold references to queues, buffers etc.
        opaqueRenderPass = nullptr;

        // Clear the contexts -> These hold swapchains and other resources
        graphicsContext = nullptr;
        uploadContext = nullptr;

        // Close all command lists
        opaqueCommandList->Close();
        transparentCommandList->Close();
        shadowCommandList->Close();
        uiCommandList->Close();

        opaqueCommandList = nullptr;
        transparentCommandList = nullptr;
        shadowCommandList = nullptr;
        uiCommandList = nullptr;

        // Destroy all resources (render targets, depth buffers, etc)
        for (auto& frame : frames)
        {
            frame = nullptr;
        }

		// Cleanup
		device = nullptr;
	}

	auto PreFrame() -> void {
		auto renderTarget = frames[frameIndex]->RenderTarget();
		auto depthBuffer = frames[frameIndex]->DepthBuffer();

        opaqueCommandList->Begin();
        transparentCommandList->Begin();
        shadowCommandList->Begin();
        uiCommandList->Begin();

        opaqueCommandList->SetRootSignature(rootSignature);
        opaqueCommandList->SetPipelineState(defaultPipelineState);
        opaqueCommandList->SetPrimitiveTopology(PrimitiveTopology::TRIANGLE_LIST);
        transparentCommandList->SetRootSignature(rootSignature);
        shadowCommandList->SetRootSignature(rootSignature);
        uiCommandList->SetRootSignature(rootSignature);

		opaqueRenderPass->Begin(opaqueCommandList, renderTarget, depthBuffer);

        graphicsContext->Begin();
        uploadContext->Begin();

        if (!textureUploaded && textureCreated) {
            uploadContext->Upload(vertexBuffer);
            uploadContext->Upload(indexBuffer);
            graphicsContext->TransitionVertexBuffer(vertexBuffer);
            graphicsContext->TransitionIndexBuffer(indexBuffer);
            uploadContext->Upload(targetTexture);
            graphicsContext->TransitionTexture(targetTexture);
            textureUploaded = true;
        }
	}

    auto Update(double deltaTime) -> void {
        uploadContext->Finish();

        if (!didUpload) {

        }

        auto cameraData = cameras[0]->GetCameraData();
        cameraBuffer->Update(&cameraData, sizeof(CameraData));

        objectData.WorldMatrix = glm::transpose(glm::rotate(glm::transpose(objectData.WorldMatrix), glm::radians(25.0f) * (float)deltaTime, glm::vec3(1, 1, 0.0f)));

        // Update buffer with new data
        objectBuffer->Update(&objectData, sizeof(ObjectData));


        opaqueCommandList->BindConstantBuffer(cameraBuffer, 0);
        opaqueCommandList->BindConstantBuffer(objectBuffer, 1);

        if (textureUploaded) {
            opaqueCommandList->BindVertexBuffer(vertexBuffer, 0);
            opaqueCommandList->BindIndexBuffer(indexBuffer);
            opaqueCommandList->BindTexture(targetTexture, 1);
        }
        opaqueCommandList->BindSampler(sampler, 2);

        opaqueCommandList->SetPrimitiveTopology(PrimitiveTopology::TRIANGLE_LIST);

		opaqueRenderPass->Execute();

        for (int x = 0; x < 1; x++) {
            opaqueCommandList->DrawIndexed(indexBuffer->Size(), 0, 0);
        }
	}

	auto PostFrame() -> void {
		opaqueRenderPass->End();

        opaqueCommandList->Close();
        transparentCommandList->Close();
        shadowCommandList->Close();
        uiCommandList->Close();

        graphicsContext->ExecuteCommandLists({ opaqueCommandList });
        graphicsContext->CopyToBackBuffer(frames[frameIndex]->RenderTarget());
		graphicsContext->Finish();

        opaqueCommandList->Reset();
        transparentCommandList->Reset();
        shadowCommandList->Reset();
        uiCommandList->Reset();

        frameIndex = (frameIndex + 1) % FRAME_COUNT;

        didUpload = true;
	}

    auto ReadbackBuffer(void* data) -> size_t {
        size_t numBytes = 0;
        graphicsContext->ReadbackBuffer(data, &numBytes);

        return numBytes;
    }

	auto LoadShader(
		const std::string& code,
		ShaderType type
	) -> uint32_t {
		//const auto shader = device->CompileShader(code, type);
		//shaders[shader->id] = shader;

        return 0;
	}

	auto UnloadShader(uint64_t shader) -> void {
		device->DestroyShader(shader);
	}

	auto CreateVertexBuffer(const void* data, size_t size, size_t stride, bool isStatic) -> VertexBufferHandle {
		auto buffer = device->CreateVertexBuffer(data, size, stride, isStatic);
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

    auto UploadMesh(const assetloader::RawMeshData& mesh) -> std::pair<VertexBufferHandle, IndexBufferHandle> {
        const UINT vertexBufferSize = mesh.vertices.size();
        const UINT indexBufferSize = mesh.indices.size();

        vertexBuffer = device->CreateVertexBuffer(mesh.vertices.data(), sizeof(Vertex) * vertexBufferSize, sizeof(Vertex), true);
        indexBuffer = device->CreateIndexBuffer(mesh.indices.data(), indexBufferSize);

        return { vertexBuffer->Id(), indexBuffer->Id() };
    }

    auto UploadTexture(const assetloader::RawTextureData& texture) -> TextureHandle {
        targetTexture = device->CreateTexture(texture.Width, texture.Height, texture.Pixels.data());

        textureCreated = true;

        return 0;
    }

	auto DrawIndexed(VertexBufferHandle vertexBuffer, IndexBufferHandle indexBuffer, MaterialHandle material) -> void {
	}

    auto CreateCamera(
        float fov,
        float aspectRatio,
        float near,
        float far,
        float pos[3],
        float rot[3],
        uint32_t renderTargetTextureId
    ) -> CameraHandle {
        cameras.push_back(std::make_unique<Camera>(fov, aspectRatio, near, far, glm::vec3(pos[0], pos[1], pos[2]), glm::quat(rot[0], rot[1], rot[2], rot[3]), 0));

        return cameras.size() - 1;
    }

    auto SetCameraFOV(
        CameraHandle handle,
        float fov
    ) -> void {
        cameras[handle]->FOV = fov;
    }

    auto SetCameraAspectRatio(
        CameraHandle handle,
        float aspectRatio
    ) -> void {
        cameras[handle]->AspectRatio = aspectRatio;
    }

    auto SetCameraNear(
        CameraHandle handle,
        float near
    ) -> void {
        cameras[handle]->Near = near;
    }

    auto SetCameraFar(
        CameraHandle handle,
        float far
    ) -> void {
        cameras[handle]->Far = far;
    }

    auto SetCameraPosition(
        CameraHandle handle,
        float pos[3]
    ) -> void {
        cameras[handle]->Position = glm::vec3(pos[0], pos[1], pos[2]);
    }

    auto SetCameraRotation(
        CameraHandle handle,
        float rot[4]
    ) -> void {
        cameras[handle]->Rotation = glm::quat(rot[0], rot[1], rot[2], rot[3]);
    }

    auto SetCameraRenderTarget(CameraHandle handle, uint32_t texture) -> void {
        cameras[handle]->RenderTargetTextureId = texture;
    }
}
