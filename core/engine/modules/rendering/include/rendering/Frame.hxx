#pragma once

#include <memory>
#include <queue>

#include "rendering/CommandAllocator.hxx"
#include "rendering/RenderTarget.hxx"
#include "rendering/DepthBuffer.hxx"
#include "rendering/ModelUploadJob.hxx"

namespace playground::rendering
{
    class Frame
    {
    public:
        Frame(
            const std::shared_ptr<RenderTarget>& renderTarget,
            const std::shared_ptr<DepthBuffer>& depth
        )
        {
            _renderTarget = renderTarget;
            _depth = depth;
        }

        ~Frame()
        {
            _renderTarget.reset();
            _depth.reset();
        }

        auto RenderTarget() const -> std::shared_ptr<rendering::RenderTarget>
        {
            return _renderTarget;
        }

        auto DepthBuffer() const -> std::shared_ptr<rendering::DepthBuffer>
        {
            return _depth;
        }

        auto ModelUploadQueue() -> std::queue<ModelUploadJob>&
        {
            return _modelUploadQueue;
        }

    private:
        std::shared_ptr<rendering::RenderTarget> _renderTarget;
        std::shared_ptr<rendering::DepthBuffer> _depth;
        std::queue<ModelUploadJob> _modelUploadQueue;
    };
}
