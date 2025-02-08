#include <SDL3/SDL.h>
#include "rendering/Context.hxx"
#include "rendering/d3d12/D3D12GraphicsContext.hxx"
#include "rendering/d3d12/D3D12GraphicsContext.hxx"
#include "rendering/d3d12/D3D12CommandAllocator.hxx"
#include "rendering/d3d12/D3D12CommandList.hxx"
#include "rendering/d3d12/D3D12RenderTarget.hxx"
#include "rendering/d3d12/D3D12IndexBuffer.hxx"
#include "rendering/d3d12/D3D12VertexBuffer.hxx"
#include "rendering/d3d12/D3D12Texture.hxx"

namespace playground::rendering::d3d12
{
    D3D12GraphicsContext::D3D12GraphicsContext(
        Microsoft::WRL::ComPtr<ID3D12Device9> device,
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> queue,
        void* window,
        uint32_t width,
        uint32_t height,
        uint32_t bufferCount,
        bool isOffscreen
    )
    {
        _device = device;
        _queue = queue;
        _bufferCount = bufferCount;
        _frameIndex = 0;

        _swapChain = std::make_unique<D3D12SwapChain>(queue, width, height, reinterpret_cast<HWND>(window));

        device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence));
        _fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

        if (FAILED(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_commandAllocator)))) {
            throw std::runtime_error("Failed to create command allocator");
        }

        if (FAILED(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _commandAllocator.Get(), nullptr, IID_PPV_ARGS(&_commandList)))) {
            throw std::runtime_error("Failed to create command list");
        }

        _commandList->SetName(L"GraphicsContextList");

        _isOffscreen = isOffscreen;

        if (_isOffscreen) {
            _readbackBuffer = std::make_unique<D3D12ReadbackBuffer>(device, width, height);
        }

        _mouseOverBuffer = std::make_unique<D3D12ReadbackBuffer>(device, 1, 1);
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

    auto D3D12GraphicsContext::TransitionIndexBuffer(std::shared_ptr<IndexBuffer> buffer) -> void {
        // Transition GPU buffer to index buffer state
        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            std::static_pointer_cast<D3D12IndexBuffer>(buffer)->Buffer().Get(),
            D3D12_RESOURCE_STATE_COPY_DEST,
            D3D12_RESOURCE_STATE_INDEX_BUFFER
        );

        _commandList->ResourceBarrier(1, &barrier);
    }

    auto D3D12GraphicsContext::TransitionVertexBuffer(std::shared_ptr<VertexBuffer> buffer) -> void {
        // Transition GPU buffer to vertex buffer state
        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            std::static_pointer_cast<D3D12VertexBuffer>(buffer)->Buffer().Get(),
            D3D12_RESOURCE_STATE_COPY_DEST,
            D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER
        );

        _commandList->ResourceBarrier(1, &barrier);
    }

    auto D3D12GraphicsContext::TransitionTexture(std::shared_ptr<Texture> texture) -> void
    {
        // Transition GPU buffer to texture state
        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            std::static_pointer_cast<D3D12Texture>(texture)->Texture().Get(),
            D3D12_RESOURCE_STATE_COPY_DEST,
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
        );

        _commandList->ResourceBarrier(1, &barrier);
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

        Microsoft::WRL::ComPtr<ID3D12Resource> backBuffer;

        if (!_isOffscreen) {
            backBuffer = _swapChain->GetBackBuffer(_frameIndex);
        }
        else {
            backBuffer = _readbackBuffer->Buffer();
        }

        auto copyFromBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
            std::static_pointer_cast<D3D12RenderTarget>(renderTarget)->Resource().Get(),
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            D3D12_RESOURCE_STATE_COPY_SOURCE
        );

        _commandList->ResourceBarrier(1, &copyFromBarrier);

        if (!_isOffscreen) {
            auto copyBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
                backBuffer.Get(),
                D3D12_RESOURCE_STATE_PRESENT,
                D3D12_RESOURCE_STATE_COPY_DEST
            );
            _commandList->ResourceBarrier(1, &copyBarrier);
        }

        // Swapchain can just copy as we are doing texture -> texture
        if (!_isOffscreen) {
            // Copy the render target to the swap chain's back buffer
            _commandList->CopyResource(backBuffer.Get(), std::static_pointer_cast<D3D12RenderTarget>(renderTarget)->Resource().Get());
        }
        // Readback buffer needs to use a copy texture region as we are doing texture -> buffer
        else {
            auto d3d312RenderTarget = std::static_pointer_cast<D3D12RenderTarget>(renderTarget)->Resource().Get();
            D3D12_RESOURCE_DESC textureDesc = d3d312RenderTarget->GetDesc();
            UINT64 totalBytes = 0;
            D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint = {};
            UINT numRows = 0;
            UINT64 rowSizeInBytes = 0;
            D3D12_SUBRESOURCE_FOOTPRINT subresourceFootprint = {};
            _device->GetCopyableFootprints(&textureDesc, 0, 1, 0, &footprint, &numRows, &rowSizeInBytes, &totalBytes);

            D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
            srcLocation.pResource = d3d312RenderTarget;
            srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
            srcLocation.SubresourceIndex = 0;

            D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
            dstLocation.pResource = _readbackBuffer->Buffer().Get();
            dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
            dstLocation.PlacedFootprint = footprint;

            _commandList->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr);
        }

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

    auto D3D12GraphicsContext::ReadbackBuffer(void* data, size_t* numBytes) -> void
    {
        if (!_isOffscreen) {
            throw std::runtime_error("Readback buffer is only supported in offscreen mode");
        }

        _readbackBuffer->Read(data, numBytes);
    }

    auto D3D12GraphicsContext::MouseOverID() -> uint64_t
    {
        uint64_t* data = nullptr;
        size_t numRead = 0;
        _mouseOverBuffer->Read(reinterpret_cast<void**>(&data), &numRead);

        return *data;
    }
}
