#pragma once

#include <memory>
#include "rendering/CommandAllocator.hxx"
#include "rendering/Constants.hxx"
#include "rendering/Device.hxx"
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
#include "rendering/ShadowMap.hxx"
#include <EASTL/deque.h>
#include <EASTL/fixed_vector.h>
#include <EASTL/vector.h>
#include <shared/Arena.hxx>
#include <sstream>

namespace playground::rendering
{
    class Frame
    {
    public:
        using VirtualArenaType = memory::VirtualArena;
        using VirtualAllocator = memory::ArenaAllocator<VirtualArenaType>;

        Frame(
            uint8_t index,
            std::shared_ptr<Device> device,
            const std::shared_ptr<RenderTarget>& renderTarget,
            const std::shared_ptr<DepthBuffer>& depth,
            std::shared_ptr<GraphicsContext> graphicsContext,
            std::shared_ptr<UploadContext> uploadContext,
            std::shared_ptr<InstanceBuffer> instanceBuffer
        ) :
            // Alloc 128 mb per frame (used for upload staging containers)
            _tempArena(128 * 1024 * 1024),
            _tempAllocator(&_tempArena, "Frame Allocator"),
            // Alloc 4 mb per frame (Used for permanent frame data)
            _arena(4 * 1024 * 1024),
            _allocator(&_arena),
            _shadowMaps(_allocator),
            _index(index),
            _device(device),
            _modelUploadQueue(_tempAllocator),
            _materialUploadQueue(_tempAllocator),
            _textureUploadQueue(_tempAllocator)
        {
            _renderTarget = renderTarget;
            _depth = depth;
            _graphicsContext = graphicsContext;
            _uploadContext = uploadContext;
            _instanceBuffer = instanceBuffer;

            std::stringstream ss;
            ss << "FRAME_" << +index << "_" << "DIRECTIONAL_LIGHT_SHADOW_MAP";

            _directionalLightShadowMap = device->CreateShadowMap(MAX_SHADOW_RES_DIRECTIONAL_LIGHT, MAX_SHADOW_RES_DIRECTIONAL_LIGHT, ss.str());
            ss.clear();
            ss = std::stringstream();

            for (int x = 0; x < MAX_SHADOW_MAPS_PER_FRAME; x++) {
                ss = std::stringstream();
                ss << "FRAME_" << +index << "_" << "SPOT_POINT_LIGHT_SHADOW_MAP_" << +x;
                _shadowMaps.push_back(device->CreateShadowMap(MAX_SHADOW_RES_POINT_AND_SPOT_LIGHT, MAX_SHADOW_RES_POINT_AND_SPOT_LIGHT, ss.str()));
                ss.clear();
            }
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

        auto DirectionalLightShadowMap() const -> std::shared_ptr<rendering::ShadowMap> {
            return _directionalLightShadowMap;
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

        auto Alloc() -> VirtualAllocator& {
            return _tempAllocator;
        }

        auto TexturesToTransition() -> std::vector<uint32_t>& const {
            return _texturesToTransition;
        }

    private:
        VirtualArenaType _tempArena;
        VirtualAllocator _tempAllocator;
        VirtualArenaType _arena;
        VirtualAllocator _allocator;

        uint8_t _index;
        std::shared_ptr<Device> _device;

        std::shared_ptr<rendering::RenderTarget> _renderTarget;
        std::shared_ptr<rendering::DepthBuffer> _depth;
        eastl::deque<ModelUploadJob, VirtualAllocator> _modelUploadQueue;
        eastl::deque<MaterialUploadJob, VirtualAllocator> _materialUploadQueue;
        eastl::deque<TextureUploadJob, VirtualAllocator> _textureUploadQueue;
        std::shared_ptr<rendering::GraphicsContext> _graphicsContext;
        std::shared_ptr<rendering::UploadContext> _uploadContext;
        std::shared_ptr<rendering::InstanceBuffer> _instanceBuffer;

        std::shared_ptr<ShadowMap> _directionalLightShadowMap;
        eastl::fixed_vector<std::shared_ptr<ShadowMap>, MAX_SHADOW_MAPS_PER_FRAME, false, VirtualAllocator> _shadowMaps;

        std::vector<uint32_t> _texturesToTransition;
    };
}
