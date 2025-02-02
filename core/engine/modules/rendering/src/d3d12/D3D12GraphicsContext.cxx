#include <SDL3/SDL.h>
#include "rendering/Context.hxx"
#include "rendering/d3d12/D3D12GraphicsContext.hxx"
#include "rendering/d3d12/D3D12CommandAllocator.hxx"
#include "rendering/d3d12/D3D12CommandList.hxx"

namespace playground::rendering::d3d12
{
    D3D12GraphicsContext::D3D12GraphicsContext(
        Microsoft::WRL::ComPtr<ID3D12Device9> device,
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> queue,
        void* window,
        uint32_t width,
        uint32_t height
    )
    {
        _device = device;
        _queue = queue;

        auto props = SDL_GetWindowProperties(static_cast<SDL_Window*>(window));
        HWND hwnd = static_cast<HWND>(SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr));

        _swapChain = std::make_unique<D3D12SwapChain>(queue, width, height, hwnd);

        device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence));
        _fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    }

    D3D12GraphicsContext::~D3D12GraphicsContext()
    {

    }

    auto D3D12GraphicsContext::Begin() -> void
    {

    }

    auto D3D12GraphicsContext::Finish() -> void
    {
        _queue->Signal(_fence.Get(), _fenceValue);

        // Wait until the GPU has finished execution
        if (_fence->GetCompletedValue() < _fenceValue) {
            _fence->SetEventOnCompletion(_fenceValue, _fenceEvent);
            WaitForSingleObject(_fenceEvent, INFINITE);
        }

        _fenceValue++;

        _swapChain->Swap();
    }

    auto D3D12GraphicsContext::ExecuteCommandLists(std::vector<std::shared_ptr<CommandList>> lists) -> void
    {

    }
}
