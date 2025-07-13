#pragma once

#include <memory>
#include <vector>
#include <string>
#include <wrl.h>
#include <directx/d3dx12.h>
#include "rendering/UploadContext.hxx"
#include "rendering/d3d12/D3D12CommandList.hxx"
#include "rendering/d3d12/D3D12IndexBuffer.hxx"
#include "rendering/d3d12/D3D12VertexBuffer.hxx"
#include "rendering/d3d12/D3D12InstanceBuffer.hxx"
#include "rendering/InstanceBuffer.hxx"
#include "rendering/d3d12/D3D12Texture.hxx"

namespace playground::rendering::d3d12 {
    class D3D12UploadContext : public UploadContext {
    public:
        D3D12UploadContext(
            std::string name,
            std::shared_ptr<D3D12Device> device,
            Microsoft::WRL::ComPtr<ID3D12CommandQueue> queue
        );
        virtual ~D3D12UploadContext() = default;

        auto Begin() -> void override;
        auto Finish() -> void override;
        auto WaitFor(const Context& other) -> void override;
        auto Upload(std::shared_ptr<Texture> texture) -> void override;
        auto Upload(std::shared_ptr<IndexBuffer> buffer) -> void override;
        auto Upload(std::shared_ptr<VertexBuffer> buffer) -> void override;
        auto Upload(std::shared_ptr<InstanceBuffer> buffer) -> void override;

        auto Fence() const -> Microsoft::WRL::ComPtr<ID3D12Fence> {
            return _fence;
        }

        auto FenceValue() const -> UINT64 {
            return _fenceValue;
        }

    private:
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> _queue;
        std::shared_ptr<D3D12CommandList> _list;
        std::vector<std::shared_ptr<Texture>> _textures;
        std::vector<std::shared_ptr<IndexBuffer>> _indexBuffers;
        std::vector<std::shared_ptr<VertexBuffer>> _vertexBuffers;
        std::vector<std::shared_ptr<InstanceBuffer>> _instanceBuffers;
        Microsoft::WRL::ComPtr<ID3D12Fence> _fence;
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> _commandAllocator;
        UINT64 _fenceValue = 0;
        HANDLE _fenceEvent;
    };
}
