#include <map>
#include <memory>
#include <sstream>
#include <thread>
#include "rendering/Rendering.hxx"
#include "rendering/Device.hxx"
#include "rendering/Frame.hxx"
#include "rendering/VertexBuffer.hxx"
#include "rendering/DeviceFactory.hxx"
#include "rendering/Texture.hxx"
#include "rendering/CommandQueue.hxx"
#include "rendering/GraphicsContext.hxx"
#include "rendering/Camera.hxx"
#include "rendering/Sampler.hxx"
#include <assetloader/AssetLoader.hxx>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#include <semaphore>
#include <queue>
#include <profiler/Profiler.hxx>
#include <tracy/Tracy.hpp>

namespace playground::rendering {
    struct Config {
        uint32_t Width;
        uint32_t Height;
        bool Offscreen;
    };

    struct ObjectBuffer {
        glm::mat4 WorldMatrix;
    };

    struct SimulationBuffer {
        float deltaTime;
        float sinTime;
        float cosTime;
        float timeSinceStart;
        int frameIndex;
    };

    Config config;

    // Use triple buffering for rendering (N = Render Thread, N + 1 = Idle, N + 2 = Main Thread)
    constexpr uint8_t FRAME_COUNT = 3;

	std::shared_ptr<Device> device = nullptr;
	void* window = nullptr;

    // Frame index used by the render thread
	uint8_t frameIndex = 0;
    // Frame index used by the main thread
    uint8_t logicFrameIndex = 2;
	std::vector<std::shared_ptr<Frame>> frames = {};

	// Resource management
    std::shared_ptr<Swapchain> swapchain;
	std::vector<std::shared_ptr<VertexBuffer>> vertexBuffers = {};
    std::vector<uint32_t> freeVertexBufferIds = {};
	std::vector<std::shared_ptr<IndexBuffer>> indexBuffers = {};
    std::vector<uint32_t> freeIndexBufferIds = {};
	std::vector<std::shared_ptr<Texture>> textures = {};
    std::vector<uint32_t> freeTextureIds = {};
	std::vector<std::shared_ptr<Shader>> shaders = {};
    std::vector<uint32_t> freeShaderIds = {};
	std::vector<std::shared_ptr<Material>> materials = {};
    std::vector<uint32_t> freeMaterialIds = {};
	std::vector<Mesh> meshes = {};
    std::vector<uint32_t> freeMeshIds = {};

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

    ObjectBuffer objectData;

    bool didUpload = false;

    std::shared_ptr<Sampler> sampler = nullptr;

    bool isRunning = false;
    std::thread renderThread;

	auto Init(
        void* window,
        uint32_t width,
        uint32_t height,
        bool offscreen
    ) -> void {
        // Create a device
        device = DeviceFactory::CreateDevice(RenderBackendType::D3D12, FRAME_COUNT);

#if _WIN32
        SetThreadDescription(GetCurrentThread(), L"Render Thread");
#endif

        config.Width = width;
        config.Height = height;
        config.Offscreen = offscreen;

        // Create frames (frames hold all render things that need to alter between frames)
        for (int x = 0; x < FRAME_COUNT; x++)
        {
            std::stringstream ss;
            ss << "COLOUR_" << x;

            auto rendertarget = device->CreateRenderTarget(width, height, TextureFormat::BGRA8, ss.str(), offscreen);

            ss.clear();
            ss << "DEPTH_" << x;

            auto depthBuffer = device->CreateDepthBuffer(width, height, ss.str());

            auto gfxName = "GraphicsContext_" + std::to_string(x);
            auto ulName = "UploadContext_" + std::to_string(x);

            auto graphicsContext = device->CreateGraphicsContext(gfxName, window, width, height, offscreen);
            auto uploadContext = device->CreateUploadContext(ulName);

            frames.emplace_back(std::make_shared<Frame>(rendertarget, depthBuffer, graphicsContext, uploadContext));
        }

        sampler = device->CreateSampler(TextureFiltering::Point, TextureWrapping::Clamp);

        auto camera = Camera(60, width / (float)(height), 0.1f, 100.0f, glm::vec3(0, 0, 0), glm::quat(0, 0, 0, 1), 0);
        cameras.push_back(std::make_unique<Camera>(camera));

        cameraBuffer = device->CreateConstantBuffer(&cameras[0], sizeof(CameraData), "CameraBuffer");

        objectBuffer = device->CreateConstantBuffer(nullptr, sizeof(ObjectBuffer), "ObjectBuffer");

        swapchain = device->CreateSwapchain(FRAME_COUNT, width, height, window);

        glm::mat4 scale = glm::identity<glm::mat4>();
        glm::mat4 transform = glm::translate(scale, glm::vec3(0, 0, 5.0f));
        glm::quat quaternion = glm::angleAxis(glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 rotationMatrix = glm::mat4_cast(quaternion);
        glm::mat4 finalTransform = glm::transpose(transform * rotationMatrix);
        objectData.WorldMatrix = finalTransform;

        tracy::SetThreadName("Render Thread");

        isRunning = true;

        static const char* GPU_FRAME = "GPU: Update";

        while (isRunning) {
            FrameMarkStart(GPU_FRAME);
            PreFrame();
            Update();
            PostFrame();
            FrameMarkEnd(GPU_FRAME);
        }

        device->WaitForIdleGPU();

        // Destroy all resources (render targets, depth buffers, etc)
        for (auto& frame : frames)
        {
            frame = nullptr;
        }

        sampler = nullptr;

        vertexBuffers = {};
        indexBuffers = {};

        swapchain = nullptr;
        // Cleanup
        device = nullptr;

        std::cout << "Render thread shutdown complete." << std::endl;
	}

	auto Shutdown() -> void {
        isRunning = false;
	}

	auto PreFrame() -> void {
        ZoneScopedN("RenderThread: Pre Frame");
        ZoneColor(tracy::Color::Orange);
        auto backBufferIndex = swapchain->BackBufferIndex();
        auto uploadContext = frames[backBufferIndex]->UploadContext();
        uploadContext->Begin();

        auto graphicsContext = frames[backBufferIndex]->GraphicsContext();
        graphicsContext->Begin();

        auto modelUploadQueue = frames[backBufferIndex]->ModelUploadQueue();
        if (modelUploadQueue.size() > 0) {
            auto modelUploadJob = modelUploadQueue.back();
            modelUploadQueue.pop();

            auto meshId = UploadMesh(modelUploadJob.meshes[modelUploadJob.handle]);

            uploadContext->Upload(vertexBuffers[meshId]);
            uploadContext->Upload(indexBuffers[meshId]);
        }

        uploadContext->Finish();
	}

    auto Update() -> void {
        ZoneScopedN("RenderThread: Update");
        ZoneColor(tracy::Color::Orange1);
        auto backBufferIndex = swapchain->BackBufferIndex();

        auto renderTarget = frames[backBufferIndex]->RenderTarget();
        auto depthBuffer = frames[backBufferIndex]->DepthBuffer();

        auto graphicsContext = frames[backBufferIndex]->GraphicsContext();

        graphicsContext->BeginRenderPass(RenderPass::Opaque, renderTarget, depthBuffer);

        auto cameraData = cameras[0]->GetCameraData();
        cameraBuffer->Update(&cameraData, sizeof(CameraData));

        objectBuffer->Update(&objectData, sizeof(ObjectBuffer));

        graphicsContext->EndRenderPass();
	}

	auto PostFrame() -> void {
        ZoneScopedN("RenderThread: Post Frame");
        ZoneColor(tracy::Color::Orange2);
        auto backBufferIndex = swapchain->BackBufferIndex();
        auto graphicsContext = frames[backBufferIndex]->GraphicsContext();
        graphicsContext->CopyToSwapchainBackBuffer(frames[backBufferIndex]->RenderTarget(), swapchain);
		graphicsContext->Finish();

        swapchain->Swap();

        logicFrameIndex = (backBufferIndex + 2) % FRAME_COUNT;
	}

    auto ReadbackBuffer(void* data) -> size_t {
        auto graphicsContext = frames[frameIndex]->GraphicsContext();

        size_t numBytes = 0;
        // TODO: Create readback buffer via renderer not graphcis context
        //graphicsContext->ReadbackBuffer(data, &numBytes);

        return numBytes;
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

    auto QueueUploadModel(std::vector<assetloader::RawMeshData>& meshes, uint32_t handle, std::function<void(uint32_t, std::vector<Mesh>)> callback) -> void {
        auto job = ModelUploadJob {
            .handle = handle,
            .meshes = meshes,
            .callback = callback
        };

        frames[logicFrameIndex]->ModelUploadQueue().push(job);
    }

    auto UploadMesh(const assetloader::RawMeshData& mesh) -> uint32_t {
        const UINT vertexBufferSize = mesh.vertices.size();
        const UINT indexBufferSize = mesh.indices.size();

        auto vertexBuffer = device->CreateVertexBuffer(mesh.vertices.data(), sizeof(Vertex) * vertexBufferSize, sizeof(Vertex), true);
        auto indexBuffer = device->CreateIndexBuffer(mesh.indices.data(), indexBufferSize);

        uint32_t vertexBufferId = 0;
        if (freeVertexBufferIds.size() > 0) {
            vertexBufferId = freeVertexBufferIds.back();
            vertexBuffers[vertexBufferId] = vertexBuffer;
            freeVertexBufferIds.pop_back();
        } else {
            vertexBuffers.push_back(vertexBuffer);
            vertexBufferId = vertexBuffers.size() - 1;
        }

        uint32_t indexBufferId = 0;
        if (freeIndexBufferIds.size() > 0) {
            indexBufferId = freeIndexBufferIds.back();
            indexBuffers[indexBufferId] = indexBuffer;
            freeIndexBufferIds.pop_back();
        } else {
            indexBuffers.push_back(indexBuffer);
            indexBufferId = indexBuffers.size() - 1;
        }

        uint32_t meshId = 0;
        if (freeMeshIds.size() > 0) {
            meshId = freeMeshIds.back();
            meshes[meshId] = Mesh { vertexBufferId, indexBufferId };
            freeMeshIds.pop_back();
        }
        else {
            meshes.push_back(Mesh { vertexBufferId, indexBufferId });
            meshId = meshes.size() - 1;
            freeMeshIds.push_back(meshId);
        }

        return meshId;
    }

    auto UploadTexture(const assetloader::RawTextureData& texture) -> TextureHandle {

        return 0;
    }

	auto DrawIndexed(VertexBufferHandle vertexBuffer, IndexBufferHandle indexBuffer, MaterialHandle material) -> void {

	}

    auto CreateMaterial(std::string& vertexShaderCode, std::string& pixelShaderCode) -> uint32_t {
        auto material = device->CreateMaterial(vertexShaderCode, pixelShaderCode);
        return 0;
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

    auto DestroyCamera(CameraHandle handle) -> void {
        // TODO: Implement camera destruction logic
    }
}
