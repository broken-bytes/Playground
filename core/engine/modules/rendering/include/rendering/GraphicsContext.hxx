#pragma once

#include <memory>
#include "rendering/Constants.hxx"
#include "rendering/Context.hxx"
#include "rendering/Material.hxx"
#include "rendering/ReadbackBuffer.hxx"
#include "rendering/RenderPass.hxx"
#include "rendering/RenderTarget.hxx"
#include "rendering/Swapchain.hxx"
#include "rendering/IndexBuffer.hxx"
#include "rendering/VertexBuffer.hxx"
#include "rendering/InstanceBuffer.hxx"
#include "rendering/ConstantBuffer.hxx"
#include "rendering/CameraBuffer.hxx"
#include "rendering/Texture.hxx"
#include "rendering/DirectionalLight.hxx"
#include <glm/glm.hpp>

namespace playground::rendering {
    class GraphicsContext : public Context {
    public:
        virtual auto BeginRenderPass(RenderPass pass, std::shared_ptr<RenderTarget> colour, std::shared_ptr<DepthBuffer> depth) -> void = 0;
        virtual auto EndRenderPass() -> void = 0;
        virtual auto Draw(uint32_t numIndices, uint32_t startIndex, uint32_t startVertex, uint32_t numInstances, uint32_t startInstance) -> void = 0;
        virtual auto BindVertexBuffer(std::shared_ptr<VertexBuffer> buffer) -> void = 0;
        virtual auto BindIndexBuffer(std::shared_ptr<IndexBuffer> buffer) -> void = 0;
        virtual auto BindInstanceBuffer(std::shared_ptr<InstanceBuffer> buffer) -> void = 0;
        virtual auto BindCamera(uint8_t index) -> void = 0;
        virtual auto SetCameraData(std::array<CameraBuffer, MAX_CAMERA_COUNT>& cameras) -> void = 0;
        virtual auto BindMaterial(std::shared_ptr<Material> material) -> void = 0;
        virtual auto TransitionIndexBuffer(std::shared_ptr<IndexBuffer> buffer) -> void = 0;
        virtual auto TransitionVertexBuffer(std::shared_ptr<VertexBuffer> buffer) -> void = 0;
        virtual auto TransitionTexture(std::shared_ptr<Texture> texture) -> void = 0;
        virtual auto CopyToSwapchainBackBuffer(std::shared_ptr<RenderTarget> source, std::shared_ptr<Swapchain> swapchain) -> void = 0;
        virtual auto CopyToReadbackBuffer(std::shared_ptr<RenderTarget> source, std::shared_ptr<ReadbackBuffer> target) -> void = 0;
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
        virtual auto MouseOverID() -> uint64_t = 0;
    };
}
