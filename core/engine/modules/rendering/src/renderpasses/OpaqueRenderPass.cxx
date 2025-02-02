#include "rendering/renderpasses/OpaqueRenderPass.hxx"
#include "rendering/CommandList.hxx"

namespace playground::rendering
{
    OpaqueRenderPass::OpaqueRenderPass()
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
        _commandList->SetViewport(0, 0, 1280, 720, 0, 1);
        _commandList->SetScissorRect(0, 0, 1280, 720);
    }

    void OpaqueRenderPass::End()
    {
    }
}
