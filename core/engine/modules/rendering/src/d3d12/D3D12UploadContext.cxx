#include "rendering/d3d12/D3D12UploadContext.hxx"

namespace playground::rendering::d3d12 {
    D3D12UploadContext::D3D12UploadContext(
        Microsoft::WRL::ComPtr<ID3D12Device> device,
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> queue
    ) : _queue(queue)
    {
        _textures = {};
        _indexBuffers = {};
        _vertexBuffers = {};

        device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence));
        _fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

        if (FAILED(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY, IID_PPV_ARGS(&_commandAllocator)))) {
            throw std::runtime_error("Failed to create command allocator");
        }

        if (FAILED(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COPY, _commandAllocator.Get(), nullptr, IID_PPV_ARGS(&_list)))) {
            throw std::runtime_error("Failed to create command list");
        }

        _list->SetName(L"UploadCommandList");
    }

    auto D3D12UploadContext::Begin() -> void {
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
    }

    auto D3D12UploadContext::Finish() -> void {
        for (auto& buffer : _indexBuffers) {
            auto d3d12Buffer = std::static_pointer_cast<D3D12IndexBuffer>(buffer)->Buffer();
            auto uploadBuffer = std::static_pointer_cast<D3D12IndexBuffer>(buffer)->StagingBuffer();
            auto size = std::static_pointer_cast<D3D12IndexBuffer>(buffer)->View().SizeInBytes;
            // Copy upload buffer to GPU memory
            _list->CopyBufferRegion(d3d12Buffer.Get(), 0, uploadBuffer.Get(), 0, size);
        }

        for (auto& buffer : _vertexBuffers) {
            auto d3d12Buffer = std::static_pointer_cast<D3D12VertexBuffer>(buffer)->Buffer();
            auto uploadBuffer = std::static_pointer_cast<D3D12VertexBuffer>(buffer)->StagingBuffer();
            auto size = std::static_pointer_cast<D3D12VertexBuffer>(buffer)->View().SizeInBytes;
            // Copy upload buffer to GPU memory
            _list->CopyBufferRegion(d3d12Buffer.Get(), 0, uploadBuffer.Get(), 0, size);
        }

        for (auto& texture : _textures) {
            auto d3d12Texture = std::static_pointer_cast<D3D12Texture>(texture)->Texture();
            auto textureUploadBuffer = std::static_pointer_cast<D3D12Texture>(texture)->StagingBuffer();
            auto textureData = std::static_pointer_cast<D3D12Texture>(texture)->TextureData();
            UpdateSubresources(_list.Get(), d3d12Texture.Get(), textureUploadBuffer.Get(), 0, 0, 1, &textureData);
        }

        _list->Close();

        std::vector<ID3D12CommandList*> commandLists;
        commandLists.push_back(_list.Get());

        _queue->ExecuteCommandLists(commandLists.size(), commandLists.data());

        _queue->Signal(_fence.Get(), _fenceValue);

        // Wait until the GPU has finished execution
        if (_fence->GetCompletedValue() < _fenceValue) {
            _fence->SetEventOnCompletion(_fenceValue, _fenceEvent);
            WaitForSingleObject(_fenceEvent, INFINITE);
        }

        _fenceValue++;

        _commandAllocator->Reset();
        _list->Reset(_commandAllocator.Get(), nullptr);
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
}
