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
#include "rendering/DirectionalLight.hxx"
#include <math/Quaternion.hxx>
#include <math/Vector3.hxx>
#include <math/Matrix4x4.hxx>
#include <assetloader/AssetLoader.hxx>
#include <shared/RingBuffer.hxx>
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
        math::Matrix4x4 ModelMatrix;
        math::Matrix4x4 NormalMatrix;
    };

    struct SimulationBuffer {
        float deltaTime;
        float sinTime;
        float cosTime;
        float timeSinceStart;
        int frameIndex;
    };;

    Config config;

    // Use triple buffering for rendering (N = Render Thread, N + 1 = Idle, N + 2 = Main Thread)
    constexpr uint8_t FRAME_COUNT = 3;
    // The maximum number of frames to render ahead
    constexpr uint8_t MAX_AHEAD_FRAMES = 8;
    // Start the instace buffer at this size
    constexpr uint32_t MAX_OBJECTS_PER_FRAME = 8192;

    uint8_t instanceBufferGrowth = 1;

	std::shared_ptr<Device> device = nullptr;
	void* window = nullptr;

    uint8_t lastDrawnFrame = 0;

    // Frame index used by the render thread
	uint8_t frameIndex = 0;
    // Frame index used by the main thread
    uint8_t logicFrameIndex = 2;
	std::vector<std::shared_ptr<Frame>> frames = {};

    // Frames that are due to be rendered, ring buffer
    RingBuffer<RenderFrame, MAX_AHEAD_FRAMES> renderFrames = {};
    RenderFrame currentFrame = {};
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

    std::array<CameraBuffer, MAX_CAMERA_COUNT> cameras = {};

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

            auto instanceBuffer = device->CreateInstanceBuffer(131072, sizeof(ObjectBuffer));

            frames.emplace_back(std::make_shared<Frame>(rendertarget, depthBuffer, graphicsContext, uploadContext, instanceBuffer));
        }

        sampler = device->CreateSampler(TextureFiltering::Anisotropic, TextureWrapping::Repeat);

        swapchain = device->CreateSwapchain(FRAME_COUNT, width, height, window);

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
	}

	auto Shutdown() -> void {
        isRunning = false;
	}

	auto PreFrame() -> void {
        ZoneScopedN("RenderThread: Pre Frame");
        ZoneColor(tracy::Color::Orange);

        auto backBufferIndex = swapchain->BackBufferIndex();
        auto uploadContext = frames[backBufferIndex]->UploadContext();

        frames[backBufferIndex]->Alloc().arena->Reset();

        uploadContext->Begin();

        auto& modelUploadQueue = frames[backBufferIndex]->ModelUploadQueue();
        while (modelUploadQueue.size() > 0) {
            auto modelUploadJob = std::move(modelUploadQueue.back());
            modelUploadQueue.pop_back();

            UploadModel(modelUploadJob);
        }

        auto& materialUploadQueue = frames[backBufferIndex]->MaterialUploadQueue();
        while (materialUploadQueue.size() > 0) {
            auto materialUploadJob = std::move(materialUploadQueue.back());
            materialUploadQueue.pop_back();

            CreateMaterial(materialUploadJob);
        }

        auto& textureQueue = frames[backBufferIndex]->TextureUploadQueue();
        while (textureQueue.size() > 0) {
            auto job = std::move(textureQueue.back());
            textureQueue.pop_back();

            UploadTexture(job);

            frames[backBufferIndex]->TexturesToTransition().push_back(job.handle);
        }

        uploadContext->Upload(frames[backBufferIndex]->InstanceBuffer());

        uploadContext->Finish();

        modelUploadQueue = {};
        materialUploadQueue = {};
        textureQueue = {};
	}

    auto Update() -> void {
        ZoneScopedN("RenderThread: Update");
        ZoneColor(tracy::Color::Orange1);
        auto backBufferIndex = swapchain->BackBufferIndex();
        auto graphicsContext = frames[backBufferIndex]->GraphicsContext();
        graphicsContext->Begin();

        {
            ZoneScopedN("RenderThread: Transition Resources");
            auto& texTransitions = frames[backBufferIndex]->TexturesToTransition();
            for (auto textureId : texTransitions) {
                graphicsContext->TransitionTexture(textures[textureId]);
            }

            texTransitions = {};
        }

        RenderFrame nextFrame;
        if (renderFrames.dequeue(nextFrame)) {
            currentFrame = nextFrame;
        }
        else {
            nextFrame = currentFrame;
        }

        auto instanceBuffer = frames[logicFrameIndex]->InstanceBuffer();
        {
            ZoneScopedN("RenderThread: Update Instance Buffer");
            for (const auto& drawCall : nextFrame.drawCalls) {
                for (int x = 0; x < drawCall.instanceData.size(); x++) {
                    const auto& instanceData = drawCall.instanceData[x];
                    instanceBuffer->SetData(&instanceData, 1, x);
                }
            }
        }

        auto renderTarget = frames[backBufferIndex]->RenderTarget();
        auto depthBuffer = frames[backBufferIndex]->DepthBuffer();

        for (int x = 0; x < nextFrame.cameras.size(); x ++) {
            assert(x < MAX_CAMERA_COUNT && "Max cameras exceeded");

            auto& cam = nextFrame.cameras[x];
            cam.SetAspectRatio((float)config.Width / config.Height);

            auto buff = CameraBuffer{
                .ViewMatrix = cam.GetViewMatrix(),
                .ProjectionMatrix = cam.GetProjectionMatrix()
            };


            cameras[x] = buff;
        }

        // Write camera data to context
        graphicsContext->SetCameraData(cameras);
        graphicsContext->SetDirectionalLight(nextFrame.sun);

        graphicsContext->BeginRenderPass(RenderPass::Opaque, renderTarget, depthBuffer);

        graphicsContext->SetViewport(0, 0, config.Width, config.Height, 0, 1);
        graphicsContext->SetScissor(0, 0, config.Width, config.Height);

        graphicsContext->BindInstanceBuffer(frames[backBufferIndex]->InstanceBuffer());
        graphicsContext->BindCamera(0);
        if (textures.size() > 0) {
            graphicsContext->BindTexture(textures[0], 6);
            graphicsContext->BindSampler(sampler, 7);
        }
        uint32_t instanceOffet = 0;

        for (auto& drawcall : nextFrame.drawCalls) {
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

    auto QueueUploadModel(std::vector<assetloader::RawMeshData>& meshes, uint32_t handle, std::function<void(uint32_t, std::vector<Mesh>)> callback) -> void {
        auto job = ModelUploadJob {
            .handle = handle,
            .meshes = meshes,
            .callback = callback
        };

        frames[logicFrameIndex]->ModelUploadQueue().push_back(job);
    }

    auto QueueUploadMaterial(
        std::string vertexShaderCode,
        std::string pixelShaderCode,
        uint32_t handle,
        std::function<void(uint32_t, uint32_t)> callback
    ) -> void {
        auto job = MaterialUploadJob{
            .handle = handle,
            .vertexShaderBlob = vertexShaderCode,
            .pixelShaderBlob = pixelShaderCode,
            .callback = callback
        };

        frames[logicFrameIndex]->MaterialUploadQueue().push_back(job);
    }

    auto QueueUploadTexture(std::shared_ptr<assetloader::RawTextureData> texture, uint32_t handle, std::function<void(uint32_t, uint32_t)> callback) -> void {
        auto job = TextureUploadJob {
            .handle = handle,
            .rawData = std::move(texture),
            .callback = callback
        };
        frames[logicFrameIndex]->TextureUploadQueue().push_back(job);
    }

    auto UploadModel(ModelUploadJob& job) -> void {
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
            }

            auto backBufferIndex = swapchain->BackBufferIndex();
            auto uploadContext = frames[backBufferIndex]->UploadContext();

            uploadContext->Upload(vertexBuffer);
            uploadContext->Upload(indexBuffer);

            meshes.push_back(Mesh{ vertexBufferId, indexBufferId });
        }

        job.callback(job.handle, meshes);
    }

    auto UploadTexture(TextureUploadJob& job) -> void {
        auto backBufferIndex = swapchain->BackBufferIndex();

        auto texture = device->CreateTexture(job.rawData->Width, job.rawData->Height, job.rawData->MipMaps, frames[backBufferIndex]->Alloc());

        uint32_t textureId = 0;
        if (freeTextureIds.size() > 0) {
            textureId = freeTextureIds.back();
            textures[textureId] = texture;
            freeTextureIds.pop_back();
        }
        else {
            textures.push_back(texture);
            textureId = textures.size() - 1;
        }

        auto uploadContext = frames[backBufferIndex]->UploadContext();
        uploadContext->Upload(texture);

        job.callback(job.handle, textureId);
    }

    auto SubmitFrame(RenderFrame frame) -> void {
        if (!isRunning) {
            return;
        }

        renderFrames.enqueue(frame);
    }

    auto CreateMaterial(MaterialUploadJob job) -> void {
        auto material = device->CreateMaterial(job.vertexShaderBlob, job.pixelShaderBlob);

        uint32_t materialId = 0;
        if (freeMaterialIds.size() > 0) {
            materialId = freeMaterialIds.back();
            materials[materialId] = material;
            freeMaterialIds.pop_back();
        }
        else {
            materials.push_back(material);
            materialId = materials.size() - 1;
        }

        job.callback(job.handle, materialId);
    }
}
