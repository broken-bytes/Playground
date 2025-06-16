#pragma once

#include <memory>
#include <queue>

#include "rendering/CommandAllocator.hxx"
#include "rendering/RenderTarget.hxx"
#include "rendering/DepthBuffer.hxx"
#include "rendering/GraphicsContext.hxx"
#include "rendering/ModelUploadJob.hxx"
#include "rendering/MaterialUploadJob.hxx"
#include "rendering/InstanceBuffer.hxx"
#include "rendering/UploadContext.hxx"
#include "rendering/DrawCall.hxx"
#include "rendering/RenderFrame.hxx"

namespace playground::rendering
{
    class Frame
    {
    public:
        Frame(
            const std::shared_ptr<RenderTarget>& renderTarget,
            const std::shared_ptr<DepthBuffer>& depth,
            std::shared_ptr<GraphicsContext> graphicsContext,
            std::shared_ptr<UploadContext> uploadContext,
            std::shared_ptr<InstanceBuffer> instanceBuffer
        )
        {
            _renderTarget = renderTarget;
            _depth = depth;
            _graphicsContext = graphicsContext;
            _uploadContext = uploadContext;
            _instanceBuffer = instanceBuffer;
        }

        ~Frame()
        {
            _graphicsContext.reset();
            _uploadContext.reset();
            _renderTarget.reset();
            _instanceBuffer.reset();
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

        auto MaterialUploadQueue() -> std::queue<MaterialUploadJob>&
        {
            return _materialUploadQueue;
        }

        auto GraphicsContext() -> std::shared_ptr<GraphicsContext> {
            return _graphicsContext;
        }

        auto UploadContext() -> std::shared_ptr<UploadContext> {
            return _uploadContext;
        }

        auto InstanceBuffer() -> std::shared_ptr<InstanceBuffer>
        {
            return _instanceBuffer;
        }

    private:
        std::shared_ptr<rendering::RenderTarget> _renderTarget;
        std::shared_ptr<rendering::DepthBuffer> _depth;
        std::queue<ModelUploadJob> _modelUploadQueue;
        std::queue<MaterialUploadJob> _materialUploadQueue;
        std::shared_ptr<rendering::GraphicsContext> _graphicsContext;
        std::shared_ptr<rendering::UploadContext> _uploadContext;
        std::shared_ptr<rendering::InstanceBuffer> _instanceBuffer;
    };
}
