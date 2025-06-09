#pragma once

#include <memory>
#include "rendering/Context.hxx"
#include "rendering/Material.hxx"
#include "rendering/ReadbackBuffer.hxx"
#include "rendering/RenderPass.hxx"
#include "rendering/RenderTarget.hxx"
#include "rendering/Swapchain.hxx"
#include "rendering/IndexBuffer.hxx"
#include "rendering/VertexBuffer.hxx"
#include "rendering/Texture.hxx"

namespace playground::rendering {
    class GraphicsContext : public Context {
    public:
        virtual auto BeginRenderPass(RenderPass pass, std::shared_ptr<RenderTarget> colour, std::shared_ptr<DepthBuffer> depth) -> void = 0;
        virtual auto EndRenderPass() -> void = 0;
        virtual auto BindVertexBuffer(std::shared_ptr<VertexBuffer> buffer) -> void = 0;
        virtual auto BindIndexBuffer(std::shared_ptr<IndexBuffer> buffer) -> void = 0;
        virtual auto BindInstanceBuffer(std::shared_ptr<VertexBuffer> buffer) -> void = 0;
        virtual auto BindMaterial(std::shared_ptr<Material> material) -> void = 0;
        virtual auto TransitionIndexBuffer(std::shared_ptr<IndexBuffer> buffer) -> void = 0;
        virtual auto TransitionVertexBuffer(std::shared_ptr<VertexBuffer> buffer) -> void = 0;
        virtual auto TransitionTexture(std::shared_ptr<Texture> texture) -> void = 0;
        virtual auto CopyToSwapchainBackBuffer(std::shared_ptr<RenderTarget> source, std::shared_ptr<Swapchain> swapchain) -> void = 0;
        virtual auto CopyToReadbackBuffer(std::shared_ptr<RenderTarget> source, std::shared_ptr<ReadbackBuffer> target) -> void = 0;
        virtual auto MouseOverID() -> uint64_t = 0;
    };
}
