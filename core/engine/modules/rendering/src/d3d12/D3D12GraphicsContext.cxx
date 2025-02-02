#include <SDL3/SDL.h>
#include "rendering/Context.hxx"
#include "rendering/d3d12/D3D12GraphicsContext.hxx"
#include "rendering/d3d12/D3D12GraphicsContext.hxx"
#include "rendering/d3d12/D3D12CommandAllocator.hxx"
#include "rendering/d3d12/D3D12CommandList.hxx"
#include "rendering/d3d12/D3D12RenderTarget.hxx"

namespace playground::rendering::d3d12
{
    D3D12GraphicsContext::D3D12GraphicsContext(
        Microsoft::WRL::ComPtr<ID3D12Device9> device,
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> queue,
        void* window,
        uint32_t width,
        uint32_t height,
        uint32_t bufferCount
    )
    {
        _device = device;
        _queue = queue;
        _bufferCount = bufferCount;
        _frameIndex = 0;

        auto props = SDL_GetWindowProperties(static_cast<SDL_Window*>(window));
        HWND hwnd = static_cast<HWND>(SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr));

        _swapChain = std::make_unique<D3D12SwapChain>(queue, width, height, hwnd);


        device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence));
        _fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

        if (FAILED(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_commandAllocator)))) {
            throw std::runtime_error("Failed to create command allocator");
        }

        if (FAILED(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _commandAllocator.Get(), nullptr, IID_PPV_ARGS(&_commandList)))) {
            throw std::runtime_error("Failed to create command list");
        }

        _commandList->SetName(L"FrameBufferSwapchainCopyList");
    }

    D3D12GraphicsContext::~D3D12GraphicsContext()
    {
        _queue->Signal(_fence.Get(), _fenceValue);

        // Wait until the GPU has finished execution
        if (_fence->GetCompletedValue() < _fenceValue) {
            _fence->SetEventOnCompletion(_fenceValue, _fenceEvent);
            WaitForSingleObject(_fenceEvent, INFINITE);
        }
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

        _frameIndex = (_frameIndex + 1) % _bufferCount;

        _commandAllocator->Reset();
        _commandList->Reset(_commandAllocator.Get(), nullptr);
    }

    auto D3D12GraphicsContext::ExecuteCommandLists(std::vector<std::shared_ptr<CommandList>> lists) -> void
    {
        std::vector<ID3D12CommandList*> commandLists;
        for (auto& list : lists) {
            commandLists.push_back(std::static_pointer_cast<D3D12CommandList>(list)->Native().Get());
        }
        _queue->ExecuteCommandLists(commandLists.size(), commandLists.data());
    }

    auto D3D12GraphicsContext::CopyToBackBuffer(std::shared_ptr<RenderTarget> renderTarget) -> void
    {
        auto backBuffer = _swapChain->GetBackBuffer(_frameIndex);

        auto copyFromBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
            std::static_pointer_cast<D3D12RenderTarget>(renderTarget)->Resource().Get(),
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            D3D12_RESOURCE_STATE_COPY_SOURCE
        );

        _commandList->ResourceBarrier(1, &copyFromBarrier);

        auto copyBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
            backBuffer.Get(),
            D3D12_RESOURCE_STATE_PRESENT,
            D3D12_RESOURCE_STATE_COPY_DEST
        );
        _commandList->ResourceBarrier(1, &copyBarrier);

        // Copy the render target to the swap chain's back buffer
        _commandList->CopyResource(backBuffer.Get(), std::static_pointer_cast<D3D12RenderTarget>(renderTarget)->Resource().Get());

        auto presentBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
            backBuffer.Get(),
            D3D12_RESOURCE_STATE_COPY_DEST,
            D3D12_RESOURCE_STATE_PRESENT
        );

        // Transition the back buffer back to PRESENT state
        _commandList->ResourceBarrier(1, &presentBarrier);

        auto renderTargetBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
            std::static_pointer_cast<D3D12RenderTarget>(renderTarget)->Resource().Get(),
            D3D12_RESOURCE_STATE_COPY_SOURCE,
            D3D12_RESOURCE_STATE_RENDER_TARGET
        );

        _commandList->ResourceBarrier(1, &renderTargetBarrier);

        _commandList->Close();
        std::vector<ID3D12CommandList*> commandLists;
        commandLists.push_back(_commandList.Get());
        _queue->ExecuteCommandLists(1, commandLists.data());
    }
}
