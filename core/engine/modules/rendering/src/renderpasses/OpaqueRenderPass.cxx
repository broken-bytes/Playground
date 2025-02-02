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

    }

    void OpaqueRenderPass::End()
    {

    }
}
