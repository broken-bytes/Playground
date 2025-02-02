#pragma once

#include <memory>
#include <wrl.h>
#include <directx/d3dx12.h>
#include "rendering/d3d12/D3D12SwapChain.hxx"
#include "rendering/d3d12/D3D12Device.hxx"
#include "rendering/d3d12/D3D12CommandQueue.hxx"
#include "rendering/Context.hxx"

namespace playground::rendering::d3d12 {
    class D3D12GraphicsContext : public rendering::Context {
        public:
        D3D12GraphicsContext(
            Microsoft::WRL::ComPtr<ID3D12Device9> device,
            Microsoft::WRL::ComPtr<ID3D12CommandQueue> queue,
            void* window,
            uint32_t width,
            uint32_t height
        );
        ~D3D12GraphicsContext();

        auto Begin() -> void override;
        auto Finish() -> void override;
        auto ExecuteCommandLists(std::vector<std::shared_ptr<CommandList>> lists) -> void override;

    private:
        Microsoft::WRL::ComPtr<ID3D12Device9> _device;
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> _queue;
        std::unique_ptr<D3D12SwapChain> _swapChain;
        Microsoft::WRL::ComPtr<ID3D12Fence> _fence;
        UINT64 _fenceValue = 1;
        HANDLE _fenceEvent;
    };
}
