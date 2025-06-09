#pragma once

#include <cstdint>
#include <directx/d3d12.h>
#include <dxgi1_5.h>
#include <wrl.h>
#include "rendering/Swapchain.hxx"
#include "rendering/RenderTarget.hxx"

namespace playground::rendering::d3d12
{
    class D3D12SwapChain : public Swapchain
    {
    public:
        D3D12SwapChain(uint8_t frameCount, Microsoft::WRL::ComPtr<ID3D12CommandQueue> queue, uint32_t width, uint32_t height, HWND hwnd);
        ~D3D12SwapChain();
        auto Swap() -> void override;
        auto GetCurrentBackBuffer() -> Microsoft::WRL::ComPtr<ID3D12Resource>;
        auto GetBackBuffer(uint8_t index) -> Microsoft::WRL::ComPtr<ID3D12Resource>;
        auto BackBufferIndex() -> uint8_t override;

    private:
        Microsoft::WRL::ComPtr<IDXGISwapChain4> _swapChain;
        uint8_t _frameCount;
    };
}
