#pragma once

#include "rendering/CameraBuffer.hxx"
#include "rendering/RenderPass.hxx"
#include "rendering/ReadbackBuffer.hxx"
#include "rendering/DirectionalLight.hxx"
#include "rendering/ShadowCaster.hxx"
#include "rendering/d3d12/D3D12CommandAllocator.hxx"
#include "rendering/d3d12/D3D12CommandList.hxx"
#include "rendering/d3d12/D3D12Device.hxx"
#include "rendering/d3d12/D3D12CommandQueue.hxx"
#include "rendering/d3d12/D3D12Swapchain.hxx"
#include "rendering/GraphicsContext.hxx"
#include "rendering/d3d12/D3D12ReadbackBuffer.hxx"
#include "rendering/d3d12/D3D12ConstantBuffer.hxx"
#include "rendering/d3d12/D3D12StructuredBuffer.hxx"
#include "rendering/d3d12/D3D12ShadowMap.hxx"
#include <array>
#include <memory>
#include <wrl.h>
#include <directx/d3dx12.h>
#include <tracy/Tracy.hpp>
#include <tracy/TracyD3D12.hpp>
#include <EASTL/vector.h>
#include <EASTL/unordered_map.h>
#include <shared/Arena.hxx>

namespace playground::rendering::d3d12 {
    class D3D12GraphicsContext : public rendering::GraphicsContext {
        public:
        D3D12GraphicsContext(
            std::string name,
            std::shared_ptr<D3D12Device> device,
            Microsoft::WRL::ComPtr<ID3D12RootSignature> skyboxRootSignature,
            Microsoft::WRL::ComPtr<ID3D12RootSignature> opaqueRootSignature,
            Microsoft::WRL::ComPtr<ID3D12RootSignature> shadowsRootSignature,
            Microsoft::WRL::ComPtr<ID3D12CommandQueue> graphicsQueue,
            Microsoft::WRL::ComPtr<ID3D12CommandQueue> transferQueue,
            void* window,
            uint32_t width,
            uint32_t height,
            bool isOffscreen
#if ENABLE_PROFILER
            ,tracy::D3D12QueueCtx* ctx
#endif
        );

        ~D3D12GraphicsContext();

        auto Begin() -> void override;
        auto BeginRenderPass(RenderPass pass, std::shared_ptr<RenderTarget> colour, std::shared_ptr<DepthBuffer> depth) -> void override;
        auto EndRenderPass() -> void override;
        auto Finish() -> void override;
        auto WaitFor(const Context& other) -> void override;
        auto Draw(uint32_t numIndices, uint32_t startIndex, uint32_t startVertex, uint32_t numInstances, uint32_t startInstance) -> void override;
        auto BindVertexBuffer(std::shared_ptr<VertexBuffer> buffer) -> void override;
        auto BindIndexBuffer(std::shared_ptr<IndexBuffer> buffer) -> void override;
        auto BindInstanceBuffer(std::shared_ptr<InstanceBuffer> buffer) -> void override;
        auto BindCamera(uint8_t index) -> void override;
        auto SetCameraData(std::array<CameraBuffer, MAX_CAMERA_COUNT>& cameras) -> void override;
        auto SetShadowCastersData(std::vector<ShadowCaster>& shadowCasters) -> void override;
        auto BindHeaps(std::vector<std::shared_ptr<Heap>> heaps) -> void override;
        auto BindMaterial(std::shared_ptr<Material>) -> void override;
        auto BindShadowMaterial(std::shared_ptr<Material> material) -> void override;
        auto BindSRVHeapToSlot(std::shared_ptr<Heap> heap, uint8_t slot) -> void override;
        auto BindSampler(std::shared_ptr<Sampler> sampler, uint8_t slot) -> void override;
        auto TransitionIndexBuffer(std::shared_ptr<IndexBuffer> buffer) -> void override;
        auto TransitionVertexBuffer(std::shared_ptr<VertexBuffer> buffer) -> void override;
        auto TransitionTexture(std::shared_ptr<Texture> texture) -> void override;
        auto TransitionCubemap(std::shared_ptr<Cubemap> cubemap) -> void override;
        auto TransitionShadowMapToDepthBuffer(std::shared_ptr<ShadowMap> map) -> void override;
        auto TransitionShadowMapToPixelShader(std::shared_ptr<ShadowMap> map) -> void override;
        auto CopyToSwapchainBackBuffer(std::shared_ptr<RenderTarget> source, std::shared_ptr<Swapchain> swapchain) -> void override;
        auto CopyToReadbackBuffer(std::shared_ptr<RenderTarget> source, std::shared_ptr<ReadbackBuffer> target) -> void override;
        auto SetMaterialData(uint32_t materialId, std::shared_ptr<Material> material) -> void override;
        auto SetDirectionalLight(
            DirectionalLight& light
        ) -> void override;
        auto SetViewport(
            uint32_t startX,
            uint32_t startY,
            uint32_t width,
            uint32_t height,
            uint32_t depthStart,
            uint32_t depthEnd
        ) -> void override;
        auto SetScissor(
            uint32_t left,
            uint32_t top,
            uint32_t right,
            uint32_t bottom
        ) -> void override;
        auto SetResolution(uint32_t width, uint32_t height) -> void override;

        auto MouseOverID() -> uint64_t override;

    private:
        using StackArenaType = memory::StackArena<4096>;
        using StackAllocator = memory::ArenaAllocator<StackArenaType>;
        using HeapArenaType = memory::VirtualArena;
        using HeapAllocator = memory::ArenaAllocator<HeapArenaType>;
        using MaterialBufferMap = eastl::unordered_map<uint32_t, std::shared_ptr<D3D12ConstantBuffer>, eastl::hash<uint32_t>, eastl::equal_to<uint32_t>, HeapAllocator>;

        StackArenaType _arena = StackArenaType();
        StackAllocator _stackAllocator = StackAllocator(&_arena, "Graphics Context Stack Allocator");
        HeapArenaType _heapArena = HeapArenaType(1024 * 1024 * 10); // 16Mb for material buffers. 1 Material = max 1kb -> Max 16K+ materials
        HeapAllocator _heapAllocator = HeapAllocator(&_heapArena, "Graphics Context Heap Allocator");

        std::string _name;
        std::shared_ptr<D3D12Device> _device;
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> _graphicsQueue;
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> _transferQueue;
        Microsoft::WRL::ComPtr<ID3D12Fence> _fence;
        Microsoft::WRL::ComPtr<ID3D12RootSignature> _skyboxRootSignature;
        Microsoft::WRL::ComPtr<ID3D12RootSignature> _opaqueRootSignature;
        Microsoft::WRL::ComPtr<ID3D12RootSignature> _shadowsRootSignature;
        std::shared_ptr<D3D12CommandAllocator> _commandAllocator;
        std::shared_ptr<D3D12CommandList> _opaqueCommandList;
        std::shared_ptr<D3D12CommandList> _transparentCommandList;
        std::shared_ptr<D3D12CommandList> _shadowCommandList;
        std::shared_ptr<D3D12CommandList> _transferCommandList;
        std::shared_ptr<D3D12CommandList> _currentPassList;
        std::shared_ptr<D3D12ConstantBuffer> _cameraBuffer;
        std::shared_ptr<D3D12StructuredBuffer> _pointLightsBuffer;
        std::shared_ptr<D3D12ConstantBuffer> _directionalLightBuffer;
        std::shared_ptr<D3D12StructuredBuffer> _shadowCastersBuffer;
        uint8_t _shadowCastersCount;
        MaterialBufferMap _materialBuffers;

        UINT64 _fenceValue = 0;
        HANDLE _fenceEvent;

        uint32_t _width;
        uint32_t _height;
        bool _isOffscreen;
        std::unique_ptr<D3D12ReadbackBuffer> _mouseOverBuffer;
#if ENABLE_PROFILER
        tracy::D3D12QueueCtx* _tracyCtx;
#endif

        void StartSkyboxRenderPass(
            std::shared_ptr<RenderTarget> colour
        );

        void StartShadowRenderPass(
            std::shared_ptr<DepthBuffer> depth
        );

        void StartOpaqueRenderPass(
            std::shared_ptr<RenderTarget> colour,
            std::shared_ptr<DepthBuffer> depth
        );
    };
}
