#pragma once

#include "rendering/Constants.hxx"
#include "rendering/Context.hxx"
#include "rendering/Heap.hxx"
#include "rendering/Material.hxx"
#include "rendering/ReadbackBuffer.hxx"
#include "rendering/RenderPass.hxx"
#include "rendering/RenderTarget.hxx"
#include "rendering/ShadowCaster.hxx"
#include "rendering/Swapchain.hxx"
#include "rendering/IndexBuffer.hxx"
#include "rendering/VertexBuffer.hxx"
#include "rendering/InstanceBuffer.hxx"
#include "rendering/ConstantBuffer.hxx"
#include "rendering/CameraBuffer.hxx"
#include "rendering/Texture.hxx"
#include "rendering/DirectionalLight.hxx"
#include "rendering/Cubemap.hxx"
#include <array>
#include <memory>

namespace playground::rendering {
    class UploadContext;

    class GraphicsContext : public Context {
    public:
        virtual auto BeginRenderPass(RenderPass pass, std::shared_ptr<RenderTarget> colour, std::shared_ptr<DepthBuffer> depth) -> void = 0;
        virtual auto EndRenderPass() -> void = 0;
        virtual auto Draw(uint32_t numIndices, uint32_t startIndex, uint32_t startVertex, uint32_t numInstances, uint32_t startInstance) -> void = 0;
        virtual auto Draw(uint32_t numVertices) -> void = 0;
        virtual auto BindVertexBuffer(std::shared_ptr<VertexBuffer> buffer) -> void = 0;
        virtual auto BindIndexBuffer(std::shared_ptr<IndexBuffer> buffer) -> void = 0;
        virtual auto BindInstanceBuffer(std::shared_ptr<InstanceBuffer> buffer) -> void = 0;
        virtual auto BindCamera(uint8_t index) -> void = 0;
        virtual auto SetCameraData(std::array<CameraBuffer, MAX_CAMERA_COUNT>& cameras) -> void = 0;
        virtual auto SetShadowCastersData(std::vector<ShadowCaster>& shadowCasters) -> void = 0;
        virtual auto BindHeaps(std::vector<std::shared_ptr<Heap>> heaps) -> void = 0;
        virtual auto BindMaterial(std::shared_ptr<Material> material) -> void = 0;
        virtual auto BindShadowMaterial(std::shared_ptr<Material> material) -> void = 0;
        virtual auto BindSRVHeapToSlot(std::shared_ptr<Heap> heap, uint8_t slot) -> void = 0;
        virtual auto BindSampler(std::shared_ptr<Sampler> sampler, uint8_t slot) -> void = 0;
        virtual auto TransitionIndexBuffer(std::shared_ptr<IndexBuffer> buffer) -> void = 0;
        virtual auto TransitionVertexBuffer(std::shared_ptr<VertexBuffer> buffer) -> void = 0;
        virtual auto TransitionTexture(std::shared_ptr<Texture> texture) -> void = 0;
        virtual auto TransitionCubemap(std::shared_ptr<Cubemap> cubemap) -> void = 0;
        virtual auto TransitionShadowMapToDepthWrite(std::shared_ptr<ShadowMap> map) -> void = 0;
        virtual auto TransitionShadowMapToPixelShader(std::shared_ptr<ShadowMap> map) -> void = 0;
        virtual auto TransitionDepthBufferToDepthWrite(std::shared_ptr<DepthBuffer> depth) -> void = 0;
        virtual auto TransitionDepthBufferToPixelShader(std::shared_ptr<DepthBuffer> depth) -> void = 0;
        virtual auto CopyToSwapchainBackBuffer(std::shared_ptr<RenderTarget> source, std::shared_ptr<Swapchain> swapchain) -> void = 0;
        virtual auto CopyToReadbackBuffer(std::shared_ptr<RenderTarget> source, std::shared_ptr<ReadbackBuffer> target) -> void = 0;
        virtual auto SetMaterialData(std::shared_ptr<Material> material) -> void = 0;
        virtual auto SetDirectionalLight(
            DirectionalLight& light
        ) -> void = 0;
        virtual auto SetViewport(
            uint32_t startX,
            uint32_t startY,
            uint32_t width,
            uint32_t height,
            uint32_t depthStart,
            uint32_t depthEnd
        ) -> void = 0;
        virtual auto SetScissor(
            uint32_t left,
            uint32_t top,
            uint32_t right,
            uint32_t bottom
        ) -> void = 0;
        virtual auto SetResolution(uint32_t width, uint32_t height) -> void = 0;
        virtual auto MouseOverID() -> uint64_t = 0;
    };
}
