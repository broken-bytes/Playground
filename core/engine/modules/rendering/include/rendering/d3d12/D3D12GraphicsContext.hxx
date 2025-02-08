#pragma once

#include <memory>
#include <wrl.h>
#include <directx/d3dx12.h>
#include "rendering/d3d12/D3D12SwapChain.hxx"
#include "rendering/d3d12/D3D12Device.hxx"
#include "rendering/d3d12/D3D12CommandQueue.hxx"
#include "rendering/GraphicsContext.hxx"
#include "rendering/d3d12/D3D12ReadbackBuffer.hxx"

namespace playground::rendering::d3d12 {
    class D3D12GraphicsContext : public rendering::GraphicsContext {
        public:
        D3D12GraphicsContext(
            Microsoft::WRL::ComPtr<ID3D12Device9> device,
            Microsoft::WRL::ComPtr<ID3D12CommandQueue> queue,
            void* window,
            uint32_t width,
            uint32_t height,
            uint32_t bufferCount,
            bool isOffscreen
        );
        ~D3D12GraphicsContext();

        auto Begin() -> void override;
        auto Finish() -> void override;
        auto TransitionIndexBuffer(std::shared_ptr<IndexBuffer> buffer) -> void override;
        auto TransitionVertexBuffer(std::shared_ptr<VertexBuffer> buffer) -> void override;
        auto TransitionTexture(std::shared_ptr<Texture> texture) -> void override;
        auto ExecuteCommandLists(std::vector<std::shared_ptr<CommandList>> lists) -> void override;
        auto CopyToBackBuffer(std::shared_ptr<RenderTarget> renderTarget) -> void override;
        auto ReadbackBuffer(void* data, size_t* numBytes) -> void override;
        auto MouseOverID() -> uint64_t override;

    private:
        Microsoft::WRL::ComPtr<ID3D12Device9> _device;
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> _queue;
        std::unique_ptr<D3D12SwapChain> _swapChain;
        Microsoft::WRL::ComPtr<ID3D12Fence> _fence;
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> _commandAllocator;
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _commandList;

        UINT64 _fenceValue = 1;
        HANDLE _fenceEvent;
        uint8_t _bufferCount;
        uint8_t _frameIndex = 0;

        bool _isOffscreen;
        std::unique_ptr<D3D12ReadbackBuffer> _mouseOverBuffer;
        std::unique_ptr<D3D12ReadbackBuffer> _readbackBuffer;
    };
}
