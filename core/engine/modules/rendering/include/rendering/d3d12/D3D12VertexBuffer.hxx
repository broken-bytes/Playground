#pragma once

#include <cstdint>
#include <stdexcept>
#include <wrl.h>
#include <directx/d3dx12.h>
#include "rendering/VertexBuffer.hxx"
#include "rendering/Vertex.hxx"

namespace playground::rendering::d3d12 {
    class D3D12VertexBuffer : public rendering::VertexBuffer {
    public:
        D3D12VertexBuffer(Microsoft::WRL::ComPtr<ID3D12Device9> device, const void* data, size_t size, size_t stride) : rendering::VertexBuffer(size) {
            const UINT vertexBufferSize = (size + 255) & ~255;

            // Create a default heap resource for the vertex buffer
            D3D12_HEAP_PROPERTIES heapProps = {};
            heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

            D3D12_RESOURCE_DESC bufferDesc = {};
            bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
            bufferDesc.Width = vertexBufferSize;
            bufferDesc.Height = 1;
            bufferDesc.DepthOrArraySize = 1;
            bufferDesc.MipLevels = 1;
            bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
            bufferDesc.SampleDesc.Count = 1;
            bufferDesc.SampleDesc.Quality = 0;
            bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
            bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

            HRESULT hr = device->CreateCommittedResource(
                &heapProps,
                D3D12_HEAP_FLAG_NONE,
                &bufferDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&_vertexBuffer)
            );
            if (FAILED(hr)) {
                throw std::runtime_error("Failed to create vertex buffer resource.");
            }

            _vertexBufferView.BufferLocation = _vertexBuffer->GetGPUVirtualAddress();
            _vertexBufferView.StrideInBytes = stride;
            _vertexBufferView.SizeInBytes = vertexBufferSize;

            void* mappedData = nullptr;
            D3D12_RANGE readRange = {};
            _vertexBuffer->Map(0, &readRange, &mappedData);
            memcpy(mappedData, data, size);
            _vertexBuffer->Unmap(0, nullptr);
        };

        ~D3D12VertexBuffer() {

        }

        auto Id() const->uint64_t override {
            return 0;
        }

        void SetData(const void* data, size_t size) override  {
        }

        void Bind() const override {
        }

        auto View() const -> const D3D12_VERTEX_BUFFER_VIEW& {
            return _vertexBufferView;
        }

        auto Buffer() const -> Microsoft::WRL::ComPtr<ID3D12Resource> {
            return _vertexBuffer;
        }

    private:
        Microsoft::WRL::ComPtr<ID3D12Resource> _vertexBuffer;
        D3D12_VERTEX_BUFFER_VIEW _vertexBufferView;
    };
}
