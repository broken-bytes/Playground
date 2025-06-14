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
#include <array>
#include <future>
#include <map>
#include <memory>
#include <sstream>
#include <thread>

namespace playground::rendering {
    struct Config {
        uint32_t Width;
        uint32_t Height;
        bool Offscreen;
    };

    struct ObjectBuffer {
        glm::mat4 ModelMatrix;
    };

    struct SimulationBuffer {
        float deltaTime;
        float sinTime;
        float cosTime;
        float timeSinceStart;
        int frameIndex;
    };

    struct CameraBuffer {
        glm::mat4 ViewMatrix;
        glm::mat4 ProjectionMatrix;
    };;

    Config config;

    // Use triple buffering for rendering (N = Render Thread, N + 1 = Idle, N + 2 = Main Thread)
    constexpr uint8_t FRAME_COUNT = 3;
    // The maximum number of frames to render ahead
    constexpr uint8_t MAX_AHEAD_FRAMES = 4;
    // The maximum number of cameras
    constexpr uint8_t MAX_CAMERAS_PER_FRAME = 8;
    // Start the instace buffer at this size
    constexpr uint32_t MAX_OBJECTS_PER_FRAME = 8192;

    uint8_t instanceBufferGrowth = 1;

	std::shared_ptr<Device> device = nullptr;
	void* window = nullptr;

    // Frame index used by the render thread
	uint8_t frameIndex = 0;
    // Frame index used by the main thread
    uint8_t logicFrameIndex = 2;
	std::vector<std::shared_ptr<Frame>> frames = {};

    // Frames that are due to be rendered, ring buffer
    std::array<RenderFrame, MAX_AHEAD_FRAMES> renderFrames = {};
    std::atomic<uint32_t> frameInUseByGPU = 0;
    std::atomic<uint32_t> nextFrameIndex = 0;

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

    ObjectBuffer objectData;

    bool didUpload = false;

    std::shared_ptr<Sampler> sampler = nullptr;

    bool isRunning = false;
    std::thread renderThread;

	auto Init(
        void* window,
        uint32_t width,
        uint32_t height,
        bool offscreen,
        std::promise<void>& rendererReadyPromise
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

            auto instanceBuffer = device->CreateInstanceBuffer(8192, sizeof(ObjectBuffer));

            frames.emplace_back(std::make_shared<Frame>(rendertarget, depthBuffer, graphicsContext, uploadContext, instanceBuffer));
        }

        sampler = device->CreateSampler(TextureFiltering::Point, TextureWrapping::Clamp);

        auto camera = Camera(60, width / (float)(height), 0.1f, 100.0f, glm::vec3(0, 0, 0), glm::quat(0, 0, 0, 1), 0);
        cameras.push_back(std::make_unique<Camera>(camera));

        cameraBuffer = device->CreateConstantBuffer(&cameras[0], sizeof(CameraBuffer), MAX_CAMERAS_PER_FRAME, "CameraBuffer");

        swapchain = device->CreateSwapchain(FRAME_COUNT, width, height, window);

        glm::mat4 scale = glm::identity<glm::mat4>();
        glm::mat4 transform = glm::translate(scale, glm::vec3(0, 0, 5.0f));
        glm::quat quaternion = glm::angleAxis(glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 rotationMatrix = glm::mat4_cast(quaternion);
        glm::mat4 finalTransform = glm::transpose(transform * rotationMatrix);
        objectData.ModelMatrix = finalTransform;

        tracy::SetThreadName("Render Thread");

        isRunning = true;
        rendererReadyPromise.set_value();

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

        renderFrames = {};
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

        auto modelUploadQueue = frames[backBufferIndex]->ModelUploadQueue();
        while (modelUploadQueue.size() > 0) {
            auto modelUploadJob = std::move(modelUploadQueue.back());
            modelUploadQueue.pop();

            UploadModel(modelUploadJob);
        }

        auto materialUploadQueue = frames[backBufferIndex]->MaterialUploadQueue();
        while (materialUploadQueue.size() > 0) {
            auto materialUploadJob = std::move(materialUploadQueue.back());
            materialUploadQueue.pop();

            CreateMaterial(materialUploadJob);
        }

        uploadContext->Upload(frames[backBufferIndex]->InstanceBuffer());

        uploadContext->Finish();
        frames[backBufferIndex]->ModelUploadQueue() = {};
        frames[backBufferIndex]->MaterialUploadQueue() = {};
	}

    auto Update() -> void {
        ZoneScopedN("RenderThread: Update");
        ZoneColor(tracy::Color::Orange1);
        auto backBufferIndex = swapchain->BackBufferIndex();

        auto frameToDraw = frames[backBufferIndex]->RenderFrame();;

        auto renderTarget = frames[backBufferIndex]->RenderTarget();
        auto depthBuffer = frames[backBufferIndex]->DepthBuffer();

        auto graphicsContext = frames[backBufferIndex]->GraphicsContext();
        graphicsContext->Begin();

        graphicsContext->BeginRenderPass(RenderPass::Opaque, renderTarget, depthBuffer);

        graphicsContext->SetViewport(0, 0, config.Width, config.Height, 0, 1);
        graphicsContext->SetScissor(0, 0, config.Width, config.Height);

        CameraBuffer data;
        data.ViewMatrix = glm::transpose(cameras[0]->GetViewMatrix());
        data.ProjectionMatrix = glm::transpose((cameras[0]->GetProjectionMatrix()));

        cameraBuffer->Update(&data, sizeof(CameraBuffer));

        graphicsContext->BindInstanceBuffer(frames[backBufferIndex]->InstanceBuffer());

        uint32_t instanceOffet = 0;

        for (auto& drawcall : frameToDraw.drawCalls) {
            graphicsContext->BindVertexBuffer(vertexBuffers[drawcall.vertexBuffer]);
            graphicsContext->BindIndexBuffer(indexBuffers[drawcall.indexBuffer]);
            graphicsContext->BindMaterial(materials[drawcall.material]);

            graphicsContext->Draw(indexBuffers[drawcall.indexBuffer]->Size(), 0, 0, drawcall.instanceData.size(), instanceOffet);

            instanceOffet = instanceOffet + drawcall.instanceData.size();
        }

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

    auto QueueUploadModel(std::vector<assetloader::RawMeshData> meshes, uint32_t handle, std::function<void(uint32_t, std::vector<Mesh>)> callback) -> void {
        auto job = ModelUploadJob {
            .handle = handle,
            .meshes = meshes,
            .callback = callback
        };

        frames[logicFrameIndex]->ModelUploadQueue().push(job);
    }

    auto QueueUploadMaterial(std::string vertexShaderCode, std::string pixelShaderCode, uint32_t handle, std::function<void(uint32_t, uint32_t)> callback) -> void {
        auto job = MaterialUploadJob{
            .handle = handle,
            .vertexShaderBlob = vertexShaderCode,
            .pixelShaderBlob = pixelShaderCode,
            .callback = callback
        };

        frames[logicFrameIndex]->MaterialUploadQueue().push(job);
    }

    auto UploadModel(ModelUploadJob job) -> void {
        auto meshes = std::vector<Mesh>();
        for (auto& mesh : job.meshes) {
            const UINT vertexBufferSize = mesh.vertices.size();
            const UINT indexBufferSize = mesh.indices.size();

            auto vertexBuffer = device->CreateVertexBuffer(mesh.vertices.data(), sizeof(Vertex) * vertexBufferSize, sizeof(Vertex), true);
            auto indexBuffer = device->CreateIndexBuffer(mesh.indices.data(), indexBufferSize);

            uint32_t vertexBufferId = 0;
            if (freeVertexBufferIds.size() > 0) {
                vertexBufferId = freeVertexBufferIds.back();
                vertexBuffers[vertexBufferId] = vertexBuffer;
                freeVertexBufferIds.pop_back();
            }
            else {
                vertexBuffers.push_back(vertexBuffer);
                vertexBufferId = vertexBuffers.size() - 1;
            }

            uint32_t indexBufferId = 0;
            if (freeIndexBufferIds.size() > 0) {
                indexBufferId = freeIndexBufferIds.back();
                indexBuffers[indexBufferId] = indexBuffer;
                freeIndexBufferIds.pop_back();
            }
            else {
                indexBuffers.push_back(indexBuffer);
                indexBufferId = indexBuffers.size() - 1;
            }

            uint32_t meshId = 0;
            if (freeMeshIds.size() > 0) {
                meshId = freeMeshIds.back();
                meshes[meshId] = Mesh{ vertexBufferId, indexBufferId };
                freeMeshIds.pop_back();
            }
            else {
                meshes.push_back(Mesh{ vertexBufferId, indexBufferId });
                meshId = meshes.size() - 1;
                freeMeshIds.push_back(meshId);
            }

            auto backBufferIndex = swapchain->BackBufferIndex();
            auto uploadContext = frames[backBufferIndex]->UploadContext();

            uploadContext->Upload(vertexBuffer);
            uploadContext->Upload(indexBuffer);

            meshes.push_back(Mesh{ vertexBufferId, indexBufferId });
        }

        job.callback(job.handle, meshes);
    }

    auto UploadTexture(const assetloader::RawTextureData& texture) -> TextureHandle {

        return 0;
    }

    auto SubmitFrame(RenderFrame frame) -> void {
        if (!isRunning) {
            return;
        }

        frames[logicFrameIndex]->SetRenderFrame(frame);

        auto instanceBuffer = frames[logicFrameIndex]->InstanceBuffer();

        std::vector<ObjectBuffer> objectMatrices;
        objectMatrices.reserve(1000);

        for (const auto& drawCall : frames[logicFrameIndex]->RenderFrame().drawCalls) {
            for (const auto& instanceData : drawCall.instanceData) {
                // Compose the model matrix from position, rotation, scale
                glm::mat4 modelMatrix =
                    glm::translate(glm::mat4(1.0f), instanceData.position) *
                    glm::mat4_cast(glm::quat(instanceData.rotation)) * // convert rotation vec4 to quat if needed
                    glm::scale(glm::mat4(1.0f), instanceData.scale);

                objectMatrices.push_back({ modelMatrix });
            }
        }

        // Upload the data to the instance buffer
        instanceBuffer->SetData(objectMatrices.data(), objectMatrices.size() * sizeof(ObjectBuffer));
    }

    auto CreateMaterial(MaterialUploadJob job) -> void {
        auto material = device->CreateMaterial(job.vertexShaderBlob, job.pixelShaderBlob);

        uint32_t materialId = 0;
        if (freeVertexBufferIds.size() > 0) {
            materialId = freeMeshIds.back();
            materials[materialId] = material;
            freeMeshIds.pop_back();
        }
        else {
            materials.push_back(material);
            materialId = materials.size() - 1;
        }

        job.callback(job.handle, materialId);
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
