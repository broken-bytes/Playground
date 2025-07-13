#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <pix.h>
#include "rendering/d3d12/D3D12UploadContext.hxx"

namespace playground::rendering::d3d12 {
    D3D12UploadContext::D3D12UploadContext(
        std::string name,
        std::shared_ptr<D3D12Device> device,
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> queue
    ) : _queue(queue)
    {
        _textures = {};
        _indexBuffers = {};
        _vertexBuffers = {};

        device->GetDevice()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence));
        _fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

        auto clName = name + "_TRANSFER_COMMAND_LIST";

        _list = std::static_pointer_cast<D3D12CommandList>(device->CreateCommandList(CommandListType::Transfer, clName));

        // Close command list
        _list->Close();
    }

    auto D3D12UploadContext::Begin() -> void {
        if (_fenceValue != 0 && _fence->GetCompletedValue() < _fenceValue) {
            _fence->SetEventOnCompletion(_fenceValue, _fenceEvent);
            WaitForSingleObject(_fenceEvent, INFINITE);
        }

        PIXBeginEvent(PIX_COLOR_INDEX(1), "Begin Upload Context");
        ZoneScopedN("RenderThread: Upload CTX Begin");
        ZoneColor(tracy::Color::Purple1);

        _list->Reset();
        for (auto& buffer : _indexBuffers) {
            std::static_pointer_cast<D3D12IndexBuffer>(buffer)->Free();
        }

        for (auto& buffer : _vertexBuffers) {
            std::static_pointer_cast<D3D12VertexBuffer>(buffer)->Free();
        }

        for (auto& texture : _textures) {
            std::static_pointer_cast<D3D12Texture>(texture)->Free();
        }

        _textures.clear();
        _indexBuffers.clear();
        _vertexBuffers.clear();
        _instanceBuffers.clear();
    }

    auto D3D12UploadContext::Finish() -> void {
        ZoneScopedN("RenderThread: Upload CTX Finish");
        ZoneColor(tracy::Color::Purple3);
        auto list = _list->Native();

        for (auto& buffer : _indexBuffers) {
            ZoneScopedN("RenderThread: Copy Index Buffer");
            ZoneColor(tracy::Color::RebeccaPurple);
            auto d3d12Buffer = std::static_pointer_cast<D3D12IndexBuffer>(buffer)->Buffer();
            auto uploadBuffer = std::static_pointer_cast<D3D12IndexBuffer>(buffer)->StagingBuffer();
            auto size = std::static_pointer_cast<D3D12IndexBuffer>(buffer)->View().SizeInBytes;
            // Copy upload buffer to GPU memory
            list->CopyBufferRegion(d3d12Buffer.Get(), 0, uploadBuffer.Get(), 0, size);
        }

        for (auto& buffer : _vertexBuffers) {
            ZoneScopedN("RenderThread: Copy Vertex Buffer");
            ZoneColor(tracy::Color::RebeccaPurple);
            auto d3d12Buffer = std::static_pointer_cast<D3D12VertexBuffer>(buffer)->Buffer();
            auto uploadBuffer = std::static_pointer_cast<D3D12VertexBuffer>(buffer)->StagingBuffer();
            auto size = std::static_pointer_cast<D3D12VertexBuffer>(buffer)->View().SizeInBytes;
            // Copy upload buffer to GPU memory
            list->CopyBufferRegion(d3d12Buffer.Get(), 0, uploadBuffer.Get(), 0, size);
        }

        for (auto& buffer : _instanceBuffers) {
            ZoneScopedN("RenderThread: Copy Instance Buffer");
            ZoneColor(tracy::Color::RebeccaPurple);
            auto d3d12Buffer = std::static_pointer_cast<D3D12InstanceBuffer>(buffer)->Buffer();
            auto uploadBuffer = std::static_pointer_cast<D3D12InstanceBuffer>(buffer)->StagingBuffer();
            auto size = std::static_pointer_cast<D3D12InstanceBuffer>(buffer)->View().SizeInBytes;
            // Copy upload buffer to GPU memory
            list->CopyBufferRegion(d3d12Buffer.Get(), 0, uploadBuffer.Get(), 0, size);
        }

        for (auto& texture : _textures) {
            ZoneScopedN("RenderThread: Copy Texture");
            ZoneColor(tracy::Color::RebeccaPurple);
            auto d3d12Texture = std::static_pointer_cast<D3D12Texture>(texture)->Texture();
            auto textureUploadBuffer = std::static_pointer_cast<D3D12Texture>(texture)->StagingBuffer();
            auto textureData = std::static_pointer_cast<D3D12Texture>(texture)->TextureData();
            auto result = UpdateSubresources(
                list.Get(),
                d3d12Texture.Get(),
                textureUploadBuffer.Get(),
                0,
                0,
                static_cast<UINT>(textureData.size()),
                textureData.data()
            );

            auto x = 0;
        }

        {
            ZoneScopedN("RenderThread: Upload CTX Submit List");
            ZoneColor(tracy::Color::RebeccaPurple);
            _list->Close();

            std::vector<ID3D12CommandList*> commandLists;
            commandLists.push_back(list.Get());

            _queue->ExecuteCommandLists(commandLists.size(), commandLists.data());

            _fenceValue++;

            _queue->Signal(_fence.Get(), _fenceValue);
        }

        PIXEndEvent();
    }

    auto D3D12UploadContext::WaitFor(const Context& other) -> void {
        // No-op for D3D12UploadContext, as it does not need to wait for other contexts.
    }

    auto D3D12UploadContext::Upload(std::shared_ptr<Texture> texture) -> void {
        _textures.push_back(texture);
    }

    auto D3D12UploadContext::Upload(std::shared_ptr<IndexBuffer> buffer) -> void {
        _indexBuffers.push_back(buffer);
    }

    auto D3D12UploadContext::Upload(std::shared_ptr<VertexBuffer> buffer) -> void {
        _vertexBuffers.push_back(buffer);
    }

    auto D3D12UploadContext::Upload(std::shared_ptr<InstanceBuffer> buffer) -> void {
        _instanceBuffers.push_back(buffer);
    }
}
