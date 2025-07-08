#pragma once

#include <memory>
#include "rendering/CommandAllocator.hxx"
#include "rendering/RenderTarget.hxx"
#include "rendering/DepthBuffer.hxx"
#include "rendering/GraphicsContext.hxx"
#include "rendering/ModelUploadJob.hxx"
#include "rendering/MaterialUploadJob.hxx"
#include "rendering/TextureUploadJob.hxx"
#include "rendering/InstanceBuffer.hxx"
#include "rendering/UploadContext.hxx"
#include "rendering/DrawCall.hxx"
#include "rendering/RenderFrame.hxx"
#include <EASTL/deque.h>
#include <shared/Arena.hxx>

namespace playground::rendering
{
    class Frame
    {
    public:
        using ArenaType = memory::VirtualArena;
        using Allocator = memory::ArenaAllocator<ArenaType>;

        Frame(
            const std::shared_ptr<RenderTarget>& renderTarget,
            const std::shared_ptr<DepthBuffer>& depth,
            std::shared_ptr<GraphicsContext> graphicsContext,
            std::shared_ptr<UploadContext> uploadContext,
            std::shared_ptr<InstanceBuffer> instanceBuffer
        ) :
            // Alloc 128 mb per frame (used for upload staging containers)
            _arena(128 * 1024 * 1024),
            _allocator(&_arena, "Frame Allocator"),
            _modelUploadQueue(_allocator),
            _materialUploadQueue(_allocator),
            _textureUploadQueue(_allocator)
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

        auto ModelUploadQueue() -> eastl::deque<ModelUploadJob, Allocator>&
        {
            return _modelUploadQueue;
        }

        auto MaterialUploadQueue() -> eastl::deque<MaterialUploadJob, Allocator>&
        {
            return _materialUploadQueue;
        }

        auto TextureUploadQueue() -> eastl::deque<TextureUploadJob, Allocator>&
        {
            return _textureUploadQueue;
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

        auto Alloc() -> Allocator& {
            return _allocator;
        }

        auto TexturesToTransition() -> std::vector<uint32_t>& const {
            return _texturesToTransition;
        }

    private:
        ArenaType _arena;
        Allocator _allocator;

        std::shared_ptr<rendering::RenderTarget> _renderTarget;
        std::shared_ptr<rendering::DepthBuffer> _depth;
        eastl::deque<ModelUploadJob, Allocator> _modelUploadQueue;
        eastl::deque<MaterialUploadJob, Allocator> _materialUploadQueue;
        eastl::deque<TextureUploadJob, Allocator> _textureUploadQueue;
        std::shared_ptr<rendering::GraphicsContext> _graphicsContext;
        std::shared_ptr<rendering::UploadContext> _uploadContext;
        std::shared_ptr<rendering::InstanceBuffer> _instanceBuffer;

        std::vector<uint32_t> _texturesToTransition;
    };
}
