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
#include "rendering/ShadowCaster.hxx"
#include <math/Quaternion.hxx>
#include <math/Vector3.hxx>
#include <math/Matrix4x4.hxx>
#include <assetloader/AssetLoader.hxx>
#include <shared/RingBuffer.hxx>
#include <shared/Logger.hxx>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <chrono>
#include <semaphore>
#include <queue>
#include <profiler/Profiler.hxx>
#include <tracy/Tracy.hpp>
#include <array>
#include <future>
#include <map>
#include <memory>
#include <mutex>
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

    struct LightBuffer {
        math::Matrix4x4 ViewProjection;
        uint32_t ShadowMapId;
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
    std::mutex uploadMutex;
    std::shared_ptr<Swapchain> swapchain;
	std::vector<std::shared_ptr<VertexBuffer>> vertexBuffers = {};
    std::vector<uint32_t> freeVertexBufferIds = {};
	std::vector<std::shared_ptr<IndexBuffer>> indexBuffers = {};
    std::vector<uint32_t> freeIndexBufferIds = {};
	std::vector<std::shared_ptr<Texture>> textures = {};
    std::vector<uint32_t> freeTextureIds = {};
    std::vector<std::shared_ptr<Cubemap>> cubemaps = {};
    std::vector<uint32_t> freeCubemapIds = {};
	std::vector<std::shared_ptr<Shader>> shaders = {};
    std::vector<uint32_t> freeShaderIds = {};
	std::vector<std::shared_ptr<Material>> materials = {};
    std::vector<uint32_t> freeMaterialIds = {};
	std::vector<Mesh> meshes = {};
    std::vector<uint32_t> freeMeshIds = {};

    std::array<CameraBuffer, MAX_CAMERA_COUNT> cameras = {};
    ShadowCaster sunLight;
    std::array<ShadowCaster, MAX_SHADOW_MAPS_PER_FRAME> lights = { };

    std::shared_ptr<Material> shadowMaterial = nullptr;
    std::shared_ptr<Material> skyboxMaterial = nullptr;
    uint32_t skyboxMaterialId = 0;

    std::vector<std::shared_ptr<Material>> postprocessMaterials = {};

    bool didUpload = false;

    bool isRunning = false;
    std::thread renderThread;

    std::chrono::high_resolution_clock::time_point lastFrameTime = std::chrono::high_resolution_clock::now();
    double deltaTime = 0.0;

    void SetupBuiltinAssets();

	auto Init(
        void* window,
        uint32_t width,
        uint32_t height,
        bool offscreen,
        std::promise<void>& rendererReadyPromise
    ) -> void {
        logging::logger::SetupSubsystem("rendering");
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
            auto shadowMapsBufferName = "ShadowMapsBuffer" + std::to_string(x);

            auto graphicsContext = device->CreateGraphicsContext(gfxName, window, width, height, offscreen);
            auto uploadContext = device->CreateUploadContext(ulName);

            auto instanceBuffer = device->CreateInstanceBuffer(131072, sizeof(ObjectBuffer));

            frames.emplace_back(std::make_shared<Frame>(x, device, rendertarget, depthBuffer, graphicsContext, uploadContext, instanceBuffer));
        }

        swapchain = device->CreateSwapchain(FRAME_COUNT, width, height, window);

        tracy::SetThreadName("Render Thread");

        isRunning = true;
        rendererReadyPromise.set_value();

        static const char* GPU_FRAME = "GPU: Update";

        SetupBuiltinAssets();

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
        lastFrameTime = std::chrono::high_resolution_clock::now();
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

        TextureUploadJob* textureUploadJob = frames[backBufferIndex]->PopTextureUploadJob();
        while (textureUploadJob != nullptr) {
            UploadTexture(textureUploadJob);

            frames[backBufferIndex]->TexturesToTransition().push_back(textureUploadJob->handle);

            delete textureUploadJob;

            textureUploadJob = frames[backBufferIndex]->PopTextureUploadJob();
        }

        auto& cubemapQueue = frames[backBufferIndex]->CubemapUploadQueue();
        while (cubemapQueue.size() > 0) {
            auto job = cubemapQueue.back();
            cubemapQueue.pop_back();

            UploadCubemap(job);

            frames[backBufferIndex]->CubemapsToTransition().push_back(job.handle);
        }

        uploadContext->Upload(frames[backBufferIndex]->InstanceBuffer());

        uploadContext->Finish();

        modelUploadQueue = {};
        materialUploadQueue = {};
        cubemapQueue = {};
	}

    auto Update() -> void {
        ZoneScopedN("RenderThread: Update");
        ZoneColor(tracy::Color::Orange1);
        auto backBufferIndex = swapchain->BackBufferIndex();
        auto graphicsContext = frames[backBufferIndex]->GraphicsContext();
        graphicsContext->Begin();

        auto renderTarget = frames[backBufferIndex]->RenderTarget();
        auto depthBuffer = frames[backBufferIndex]->DepthBuffer();
        auto shadowMap = frames[backBufferIndex]->DirectionalLightShadowMap();

        {
            graphicsContext->BeginRenderPass(RenderPass::Preparation, nullptr, nullptr);
            ZoneScopedN("RenderThread: Transition Resources");
            auto& texTransitions = frames[backBufferIndex]->TexturesToTransition();
            for (auto textureId : texTransitions) {
                graphicsContext->TransitionTexture(textures[textureId]);
            }

            texTransitions = {};

            auto& cubeTransitions = frames[backBufferIndex]->CubemapsToTransition();
            for (auto cubemapId : cubeTransitions) {
                graphicsContext->TransitionCubemap(cubemaps[cubemapId]);
            }

            cubeTransitions = {};

            graphicsContext->EndRenderPass();
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
            uint16_t instanceOffset = 0;
            for (const auto& drawCall : nextFrame.drawCalls) {
                for (int x = 0; x < drawCall.instanceData.size(); x++) {
                    const auto& instanceData = drawCall.instanceData[x];
                    instanceBuffer->SetData(&instanceData, 1, instanceOffset);
                    instanceOffset++;
                }
            }
        }

        for (int x = 0; x < nextFrame.cameras.size(); x++) {
            assert(x < MAX_CAMERA_COUNT && "Max cameras exceeded");

            auto& cam = nextFrame.cameras[x];
            cam.SetAspectRatio((float)config.Width / config.Height);

            auto viewMatrix = cam.GetViewMatrix();
            auto projectionMatrix = cam.GetProjectionMatrix();
            math::Matrix4x4 inverseViewMatrix;
            math::Matrix4x4 inverseProjectionMatrix;

            math::InverseAffine(viewMatrix, &inverseViewMatrix);
            math::InverseAffine(projectionMatrix, &inverseProjectionMatrix);

            auto buff = CameraBuffer{
                .ViewMatrix = viewMatrix,
                .ProjectionMatrix = projectionMatrix,
                .InverseViewMatrix = inverseViewMatrix,
                .InverseProjectionMatrix = inverseProjectionMatrix
            };
            
            cameras[x] = buff;
        }

        // Write camera data to context
        graphicsContext->SetCameraData(cameras);
        graphicsContext->SetDirectionalLight(nextFrame.sun);

        graphicsContext->BeginRenderPass(RenderPass::Skybox, frames[backBufferIndex]->RenderTarget(), nullptr);
        graphicsContext->BindHeaps({ device->GetSrvHeap(), device->GetSamplerHeap() });
        graphicsContext->SetViewport(0, 0, config.Width, config.Height, 0, 1);
        graphicsContext->SetScissor(0, 0, config.Width, config.Height);
        graphicsContext->BindCamera(0);

        if (skyboxMaterial != nullptr) {
            graphicsContext->SetMaterialData(skyboxMaterial);
            graphicsContext->BindMaterial(skyboxMaterial);
            graphicsContext->Draw(36);
        }

        graphicsContext->EndRenderPass();

        uint32_t instanceOffet = 0;

        graphicsContext->BeginRenderPass(RenderPass::Shadow, nullptr, shadowMap->GetDepthBuffer());

        graphicsContext->BindHeaps({ device->GetSrvHeap(), device->GetSamplerHeap() });
        graphicsContext->SetViewport(0, 0, shadowMap->Width(), shadowMap->Height(), 0, 1);
        graphicsContext->SetScissor(0, 0, shadowMap->Width(), shadowMap->Height());
        graphicsContext->BindInstanceBuffer(frames[backBufferIndex]->InstanceBuffer());

        if (shadowMaterial != nullptr) {
            graphicsContext->BindShadowMaterial(shadowMaterial);
            for (auto& drawcall : nextFrame.drawCalls) {
                graphicsContext->BindVertexBuffer(vertexBuffers[drawcall.vertexBuffer]);
                graphicsContext->BindIndexBuffer(indexBuffers[drawcall.indexBuffer]);

                graphicsContext->Draw(indexBuffers[drawcall.indexBuffer]->Size(), 0, 0, drawcall.instanceData.size(), instanceOffet);

                instanceOffet = instanceOffet + drawcall.instanceData.size();
            }
        }
        graphicsContext->EndRenderPass();

        instanceOffet = 0;

        graphicsContext->BeginRenderPass(RenderPass::PostShadow, nullptr, nullptr);
        graphicsContext->TransitionShadowMapToPixelShader(shadowMap);
        graphicsContext->EndRenderPass();

        graphicsContext->BeginRenderPass(RenderPass::Opaque, renderTarget, depthBuffer);
        graphicsContext->BindHeaps({ device->GetSrvHeap(), device->GetSamplerHeap() });
        graphicsContext->BindCamera(0);
        graphicsContext->SetViewport(0, 0, config.Width, config.Height, 0, 1);
        graphicsContext->SetScissor(0, 0, config.Width, config.Height);
        graphicsContext->BindInstanceBuffer(frames[backBufferIndex]->InstanceBuffer());

        std::vector<ShadowCaster> shadowcasters = {};
        shadowcasters.reserve(MAX_SHADOW_MAPS_PER_FRAME + 1);

        shadowcasters.emplace_back(nextFrame.sun.viewMatrix, nextFrame.sun.projectionMatrix, frames[backBufferIndex]->DirectionalLightShadowMap()->ID());

        // TODO: Add other lights to the buffer
        graphicsContext->SetShadowCastersData(shadowcasters);

        for (auto& drawcall : nextFrame.drawCalls) {
            // TODO: Mark materials dirty and bulk update all dirty ones
            graphicsContext->SetMaterialData(materials[drawcall.material]);

            graphicsContext->BindVertexBuffer(vertexBuffers[drawcall.vertexBuffer]);
            graphicsContext->BindIndexBuffer(indexBuffers[drawcall.indexBuffer]);
            graphicsContext->BindMaterial(materials[drawcall.material]);

            graphicsContext->Draw(indexBuffers[drawcall.indexBuffer]->Size(), 0, 0, drawcall.instanceData.size(), instanceOffet);

            instanceOffet = instanceOffet + drawcall.instanceData.size();
        }
        graphicsContext->EndRenderPass();

        graphicsContext->BeginRenderPass(RenderPass::PostOpaque, nullptr, nullptr);
        graphicsContext->TransitionDepthBufferToPixelShader(depthBuffer);
        graphicsContext->EndRenderPass();

        for (auto& postProcessingEffect : postprocessMaterials) {
            graphicsContext->BeginRenderPass(RenderPass::PostProcessing, renderTarget, depthBuffer);
            graphicsContext->BindHeaps({ device->GetSrvHeap(), device->GetSamplerHeap() });
            graphicsContext->SetViewport(0, 0, config.Width, config.Height, 0, 1);
            graphicsContext->SetScissor(0, 0, config.Width, config.Height);
            graphicsContext->SetMaterialData(postProcessingEffect);
            graphicsContext->BindMaterial(postProcessingEffect);
            graphicsContext->BindCamera(0);
            graphicsContext->Draw(3);
            graphicsContext->EndRenderPass();
        }
	}

	auto PostFrame() -> void {
        ZoneScopedN("RenderThread: Post Frame");
        ZoneColor(tracy::Color::Orange2);
        auto backBufferIndex = swapchain->BackBufferIndex();
        auto graphicsContext = frames[backBufferIndex]->GraphicsContext();

        auto renderTarget = frames[backBufferIndex]->RenderTarget();
        auto depthBuffer = frames[backBufferIndex]->DepthBuffer();
        auto shadowMap = frames[backBufferIndex]->DirectionalLightShadowMap();

        graphicsContext->CopyToSwapchainBackBuffer(frames[backBufferIndex]->RenderTarget(), swapchain);

        graphicsContext->BeginRenderPass(RenderPass::Completion, nullptr, nullptr);
        graphicsContext->TransitionDepthBufferToDepthWrite(depthBuffer);
        graphicsContext->TransitionShadowMapToDepthWrite(shadowMap);
        graphicsContext->EndRenderPass();

        graphicsContext->WaitFor(*frames[backBufferIndex]->UploadContext().get());
		graphicsContext->Finish();

        swapchain->Swap();

        logicFrameIndex = (backBufferIndex + 2) % FRAME_COUNT;

        deltaTime = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - lastFrameTime).count();
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
        std::scoped_lock lock(uploadMutex);
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
        MaterialType type,
        uint32_t handle,
        std::function<void(uint32_t, uint32_t)> callback,
        void (*onCompletion)(uint32_t)
    ) -> void {
        std::scoped_lock lock(uploadMutex);

        auto job = MaterialUploadJob{
            .handle = handle,
            .type = type,
            .vertexShaderBlob = vertexShaderCode,
            .pixelShaderBlob = pixelShaderCode,
            .callback = callback,
            .onCompletion = onCompletion
        };

        frames[logicFrameIndex]->MaterialUploadQueue().push_back(job);
    }

    auto QueueUploadTexture(assetloader::RawTextureData* texture, uint32_t handle, void (*callback)(uint32_t, uint32_t)) -> void {
        auto job = new TextureUploadJob(
            handle,
            texture,
            callback
        );
        frames[logicFrameIndex]->PushTextureUploadJob(job);
    }

    auto QueueUploadCubemap(std::shared_ptr<assetloader::RawCubemapData> cubemap, uint32_t handle, std::function<void(uint32_t, uint32_t)> callback) -> void {
        std::scoped_lock lock(uploadMutex);

        ZoneScopedN("RenderThread: Queue Upload Cubemap");
        std::vector<std::vector<std::vector<uint8_t>>> facesRawData;
        facesRawData.reserve(cubemap->facesData.size());

        for (int face = 0; face < cubemap->facesData.size(); face++) {
            facesRawData.push_back(std::vector<std::vector<uint8_t>>());
            for(int mip = 0; mip < cubemap->facesData[face].MipMaps.size(); mip++) {
                auto& mipData = cubemap->facesData[face].MipMaps[mip];
                facesRawData[face].push_back(mipData);
            }
        }

        cubemap->facesRawData = std::move(facesRawData);
       
        auto job = CubemapUploadJob{
            .handle = handle,
            .cubemapData = std::move(cubemap),
            .callback = callback
        };

        frames[logicFrameIndex]->CubemapUploadQueue().push_back(job);
    }

    auto UploadModel(ModelUploadJob& job) -> void {
        ZoneScopedN("RenderThread: Upload Model");
        ZoneColor(tracy::Color::Violet);
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

    auto UploadTexture(TextureUploadJob* job) -> void {
        ZoneScopedN("RenderThread: Upload Texture");
        ZoneColor(tracy::Color::Violet);
        auto backBufferIndex = swapchain->BackBufferIndex();

        auto texture = device->CreateTexture(job->rawData->Width, job->rawData->Height, job->rawData->MipMaps, frames[backBufferIndex]->Alloc());

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

        job->callback(job->handle, textureId);
    }

    auto UploadCubemap(CubemapUploadJob& job) -> void {
        ZoneScopedN("RenderThread: Upload Cubemap");
        ZoneColor(tracy::Color::Violet);
        auto backBufferIndex = swapchain->BackBufferIndex();

        auto cubemap = device->CreateCubemap(job.cubemapData->Width, job.cubemapData->Height, job.cubemapData->facesRawData, frames[backBufferIndex]->Alloc());

        uint32_t cubemapId = 0;
        if (freeCubemapIds.size() > 0) {
            cubemapId = freeCubemapIds.back();
            cubemaps[cubemapId] = cubemap;
            freeCubemapIds.pop_back();
        }
        else {
            cubemaps.push_back(cubemap);
            cubemapId = cubemaps.size() - 1;
        }

        auto uploadContext = frames[backBufferIndex]->UploadContext();
        uploadContext->Upload(cubemap);

        job.callback(job.handle, cubemapId);
    }

    auto SubmitFrame(RenderFrame frame) -> void {
        if (!isRunning) {
            return;
        }

        renderFrames.enqueue(frame);
    }

    double GetGPUFrameTime() {
        return deltaTime;
    }

    auto CreateMaterial(MaterialUploadJob job) -> void {
        auto material = device->CreateMaterial(job.vertexShaderBlob, job.pixelShaderBlob, job.type);

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

        material->id = materialId;
        job.callback(job.handle, materialId);
    }

    auto RegisterShadowShader(
        std::string& vertexShaderCode
    ) -> void {
        shadowMaterial = device->CreateMaterial(vertexShaderCode, "", MaterialType::Shadow);
    }

    void RegisterPostProcessingMaterial(uint8_t slot, uint32_t materialId) {
        if (slot >= postprocessMaterials.size()) {
            postprocessMaterials.resize(slot + 1);
        }

        if (materialId >= materials.size()) {
            std::cerr << "Tried to register post processing material at slot " << (int)slot << ", but material ID " << materialId << " is out of bounds." << std::endl;
            return;
        }

        postprocessMaterials[slot] = materials[materialId];
    }

    void UnregisterPostProcessingMaterial(uint8_t slot) {
        if (slot < postprocessMaterials.size()) {
            postprocessMaterials[slot] = nullptr;
        }
        else {
            std::cerr << "Tried to unregister post processing material at slot " << (int)slot << ", but it is out of bounds." << std::endl;
        }
    }

    auto SetSkyboxMaterial(uint32_t materialId) -> void {
        skyboxMaterial = materials[materialId];
        skyboxMaterialId = materialId;
    }

    void SetMaterialTexture(uint32_t materialId, uint8_t slot, uint32_t textureId) {
        if (textureId >= textures.size()) {
            std::cerr << "Tried to set texture for material " << materialId << ", but texture ID " << textureId << " is out of bounds." << std::endl;
            return;
        }

        auto material = materials[materialId];

        if (material->textures.size() <= slot) {
            material->textures.resize(material->textures.size() + 1);
        }

        material->textures[slot] = textures[textureId]->ID();
    }

    void SetMaterialCubemap(uint32_t materialId, uint8_t slot, uint32_t cubemapId) {
        if (cubemapId >= cubemaps.size()) {
            std::cerr << "Tried to set cubemap for material " << materialId << ", but cubemap ID " << cubemapId << " is out of bounds." << std::endl;
            return;
        }
        auto material = materials[materialId];
        if (material->cubemaps.size() <= slot) {
            material->cubemaps.resize(material->cubemaps.size() + 1);
        }
        material->cubemaps[slot] = cubemaps[cubemapId]->ID();
    }

    void SetMaterialFloat(uint32_t materialId, uint8_t slot, float value) {
        auto material = materials[materialId];
        if (material->floats.size() <= slot) {
            material->floats.resize(material->floats.size() + 1);
        }
        material->floats[slot] = value;
    }

    void SetupBuiltinAssets() {
        uint8_t checkerboard[8 * 8 * 4];
        for (int y = 0; y < 8; ++y) {
            for (int x = 0; x < 8; ++x) {
                int i = (y * 8 + x) * 4;
                bool red = ((x + y) & 1) == 0;
                checkerboard[i + 0] = red ? 255 : 0;
                checkerboard[i + 1] = 0;
                checkerboard[i + 2] = red ? 0 : 255;
                checkerboard[i + 3] = 255;
            }
        }

        assetloader::RawTextureData checkerboardTextureData{
            .MipMaps = {
                std::vector<uint8_t>(checkerboard, checkerboard + sizeof(checkerboard))
            },
            .Width = 8,
            .Height = 8,
            .Channels = 4,
        };

        //QueueUploadTexture(&checkerboardTextureData, 0, [](uint32_t, uint32_t) {});
    }
}
