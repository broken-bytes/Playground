#pragma once

#include <memory>
#include <wrl.h>
#include <directx/d3dx12.h>
#include "rendering/RenderPass.hxx"
#include "rendering/ReadbackBuffer.hxx"
#include "rendering/Swapchain.hxx"
#include "rendering/d3d12/D3D12CommandAllocator.hxx"
#include "rendering/d3d12/D3D12CommandList.hxx"
#include "rendering/d3d12/D3D12Device.hxx"
#include "rendering/d3d12/D3D12CommandQueue.hxx"
#include "rendering/d3d12/D3D12Swapchain.hxx"
#include "rendering/GraphicsContext.hxx"
#include "rendering/d3d12/D3D12ReadbackBuffer.hxx"
#include "rendering/d3d12/D3D12ConstantBuffer.hxx"
#include <tracy/Tracy.hpp>
#include <tracy/TracyD3D12.hpp>


namespace playground::rendering::d3d12 {
    class D3D12GraphicsContext : public rendering::GraphicsContext {
        public:
        D3D12GraphicsContext(
            std::string name,
            std::shared_ptr<D3D12Device> device,
            Microsoft::WRL::ComPtr<ID3D12RootSignature> opaqueRootSignature,
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
        auto Draw(uint32_t numIndices, uint32_t startIndex, uint32_t startVertex, uint32_t numInstances, uint32_t startInstance) -> void override;
        auto BindVertexBuffer(std::shared_ptr<VertexBuffer> buffer) -> void override;
        auto BindIndexBuffer(std::shared_ptr<IndexBuffer> buffer) -> void override;
        auto BindInstanceBuffer(std::shared_ptr<InstanceBuffer> buffer) -> void override;
        auto BindConstantBuffer(std::shared_ptr<ConstantBuffer> buffer, uint8_t index) -> void override;
        auto BindCamera(uint8_t index) -> void override;
        auto SetCameraData(std::array<CameraBuffer, MAX_CAMERA_COUNT>& cameras) -> void override;
        auto BindMaterial(std::shared_ptr<Material>) -> void override;
        auto TransitionIndexBuffer(std::shared_ptr<IndexBuffer> buffer) -> void override;
        auto TransitionVertexBuffer(std::shared_ptr<VertexBuffer> buffer) -> void override;
        auto TransitionTexture(std::shared_ptr<Texture> texture) -> void override;
        auto CopyToSwapchainBackBuffer(std::shared_ptr<RenderTarget> source, std::shared_ptr<Swapchain> swapchain) -> void override;
        auto CopyToReadbackBuffer(std::shared_ptr<RenderTarget> source, std::shared_ptr<ReadbackBuffer> target) -> void override;
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
        auto MouseOverID() -> uint64_t override;

    private:
        Microsoft::WRL::ComPtr<ID3D12Device9> _device;
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> _graphicsQueue;
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> _transferQueue;
        Microsoft::WRL::ComPtr<ID3D12Fence> _fence;
        Microsoft::WRL::ComPtr<ID3D12RootSignature> _opaqueRootSignature;
        std::shared_ptr<D3D12CommandAllocator> _commandAllocator;
        std::shared_ptr<D3D12CommandList> _opaqueCommandList;
        std::shared_ptr<D3D12CommandList> _transparentCommandList;
        std::shared_ptr<D3D12CommandList> _shadowCommandList;
        std::shared_ptr<D3D12CommandList> _transferCommandList;
        std::shared_ptr<D3D12CommandList> _currentPassList;
        std::shared_ptr<D3D12ConstantBuffer> _cameraBuffer;

        UINT64 _fenceValue = 0;
        HANDLE _fenceEvent;

        bool _isOffscreen;
        std::unique_ptr<D3D12ReadbackBuffer> _mouseOverBuffer;
#if ENABLE_PROFILER
        tracy::D3D12QueueCtx* _tracyCtx;
#endif
    };
}
