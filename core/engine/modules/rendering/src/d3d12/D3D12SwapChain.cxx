#include <stdexcept>
#include <sstream>
#include "rendering/d3d12/D3D12SwapChain.hxx"
#include "rendering/d3d12/D3D12RenderTarget.hxx"

namespace playground::rendering::d3d12
{
    D3D12SwapChain::D3D12SwapChain(uint8_t frameCount, Microsoft::WRL::ComPtr<ID3D12CommandQueue> queue, uint32_t width, uint32_t height, HWND hwnd)
    {
        UINT dxgiFactoryFlags = 0;
        Microsoft::WRL::ComPtr<IDXGIFactory4> factory;
        CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory));

        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.Width = width;
        swapChainDesc.Height = height;
        swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        swapChainDesc.Stereo = FALSE;
        swapChainDesc.SampleDesc = { 1, 0 };
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount = frameCount;
        swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
        swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;

        Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain;

        swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
        if (FAILED(factory->CreateSwapChainForHwnd(
            queue.Get(),
            hwnd,
            &swapChainDesc,
            nullptr,
            nullptr,
            &swapChain
        )))
        {
            throw std::runtime_error("failed to create swap chain");
        }

        if (FAILED(swapChain.As(&_swapChain)))
        {
            throw std::runtime_error("Failed to cast swap chain");
        }

        _frameCount = frameCount;

        for (int x = 0; x < frameCount; x++) {
            std::wstringstream ss;
            ss << "BACK_BUFFER_" << x;
            GetBackBuffer(x)->SetName(ss.str().c_str());
        }
    }

    D3D12SwapChain::~D3D12SwapChain()
    {

    }

    auto D3D12SwapChain::Swap() -> void
    {
        _swapChain->Present(1, 0);
    }

    auto D3D12SwapChain::GetCurrentBackBuffer() -> Microsoft::WRL::ComPtr<ID3D12Resource> {
        UINT idx = _swapChain->GetCurrentBackBufferIndex();
        return GetBackBuffer(idx);
    }

    auto D3D12SwapChain::GetBackBuffer(uint8_t index) -> Microsoft::WRL::ComPtr<ID3D12Resource> {
        Microsoft::WRL::ComPtr<ID3D12Resource> backBuffer;
        _swapChain->GetBuffer(index, IID_PPV_ARGS(&backBuffer));

        return backBuffer;
    }

    auto D3D12SwapChain::BackBufferIndex() -> uint8_t {
        return _swapChain->GetCurrentBackBufferIndex();
    }
}
