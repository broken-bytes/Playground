#include "rendering/renderpasses/OpaqueRenderPass.hxx"
#include "rendering/CommandList.hxx"

namespace playground::rendering
{
    OpaqueRenderPass::OpaqueRenderPass(uint32_t width, uint32_t height) : _width(width), _height(height)
    {

    }

    OpaqueRenderPass::~OpaqueRenderPass()
    {
    }


    void OpaqueRenderPass::Begin(std::shared_ptr<CommandList> commandList, std::shared_ptr<RenderTarget> colour, std::shared_ptr<DepthBuffer> depth)
    {
        _commandList = commandList;
        _colour = colour;
        _depth = depth;
    }

    void OpaqueRenderPass::Execute()
    {
        _commandList->ClearRenderTarget(_colour, { 0.2f, 0.0f, 0.7f, 1.0f });
        _commandList->ClearDepthTarget(_depth, 1.0f);
        _commandList->SetRenderTarget(_colour, _depth);
        _commandList->SetViewport(0, 0, _width, _height, 0, 1);
        _commandList->SetScissorRect(0, 0, _width, _height);
    }

    void OpaqueRenderPass::End()
    {
    }
}
