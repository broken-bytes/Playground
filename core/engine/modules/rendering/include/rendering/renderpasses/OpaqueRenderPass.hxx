#pragma once

#include <memory>
#include "rendering/RenderPass.hxx"
#include "rendering/Device.hxx"
#include "rendering/RenderTarget.hxx"
#include "rendering/DepthBuffer.hxx"

namespace playground::rendering
{
    class OpaqueRenderPass: public RenderPass
    {
    public:
        OpaqueRenderPass();
        ~OpaqueRenderPass() final;
        void Begin(std::shared_ptr<CommandList> list, std::shared_ptr<RenderTarget> colour, std::shared_ptr<DepthBuffer> depth) override;
        void Execute() override;
        void End() override;

    private:
        std::shared_ptr<CommandList> _commandList;
        std::shared_ptr<RenderTarget> _colour;
        std::shared_ptr<DepthBuffer> _depth;
    };
}
