#pragma once

#include <cstdint>
#include <stdexcept>
#include <wrl.h>
#include <directx/d3dx12.h>
#include "rendering/IndexBuffer.hxx"

namespace playground::rendering::d3d12 {
    class D3D12IndexBuffer : public rendering::IndexBuffer {
    private:
        auto FillBuffer(
            const void* data,
            size_t size,
            Microsoft::WRL::ComPtr<ID3D12Device9> device,
            D3D12_RESOURCE_DESC bufferDesc,
            Microsoft::WRL::ComPtr<ID3D12Resource>& buffer
        ) {
            // Create a default heap resource for the index buffer
            D3D12_HEAP_PROPERTIES heapProps = {};
            heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

            HRESULT hr = device->CreateCommittedResource(
                &heapProps,
                D3D12_HEAP_FLAG_NONE,
                &bufferDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&buffer)
            );
            if (FAILED(hr)) {
                throw std::runtime_error("Failed to create index buffer resource.");
            }

            void* mappedData = nullptr;
            D3D12_RANGE readRange = {};
            buffer->Map(0, &readRange, &mappedData);
            memcpy(mappedData, data, size);
        }

        auto PrepareStaticBuffer(
            Microsoft::WRL::ComPtr<ID3D12Device9> device,
            D3D12_RESOURCE_DESC bufferDesc,
            Microsoft::WRL::ComPtr<ID3D12Resource>& buffer
        ) {
            // Create a default heap resource for the vertex buffer
            D3D12_HEAP_PROPERTIES heapProps = {};
            heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

            HRESULT hr = device->CreateCommittedResource(
                &heapProps,
                D3D12_HEAP_FLAG_NONE,
                &bufferDesc,
                D3D12_RESOURCE_STATE_COMMON,
                nullptr,
                IID_PPV_ARGS(&buffer)
            );
            if (FAILED(hr)) {
                throw std::runtime_error("Failed to create index buffer resource.");
            }
        }

    public:
        D3D12IndexBuffer(Microsoft::WRL::ComPtr<ID3D12Device9> device, const uint32_t* data, size_t size) : rendering::IndexBuffer(size) {
            // Create a buffer in the upload heap
            D3D12_HEAP_PROPERTIES heapProps = {};
            heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

            const uint32_t bufferSize = size * sizeof(uint32_t);

            D3D12_RESOURCE_DESC bufferDesc = {};
            bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
            bufferDesc.Width = bufferSize; // Exact size of the index buffer
            bufferDesc.Height = 1;
            bufferDesc.DepthOrArraySize = 1;
            bufferDesc.MipLevels = 1;
            bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
            bufferDesc.SampleDesc.Count = 1;
            bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
            bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

            HRESULT hr = device->CreateCommittedResource(
                &heapProps,
                D3D12_HEAP_FLAG_NONE,
                &bufferDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&_indexBuffer)
            );
            if (FAILED(hr)) {
                throw std::runtime_error("Failed to create index buffer.");
            }

            FillBuffer(data, bufferSize, device, bufferDesc, _stagingBuffer);
            PrepareStaticBuffer(device, bufferDesc, _indexBuffer);
            _stagingBuffer->Unmap(0, nullptr);        

            // Create the index buffer view
            _indexBufferView.BufferLocation = _indexBuffer->GetGPUVirtualAddress();
            _indexBufferView.SizeInBytes = bufferSize;
            _indexBufferView.Format = DXGI_FORMAT_R32_UINT;       
        };

        auto Id() const->uint64_t override {
            return 0;
        }

        void SetData(const void* data, size_t size) override {
        }

        void Bind() const override {
        }

        auto View() const -> const D3D12_INDEX_BUFFER_VIEW& {
            return _indexBufferView;
        }

        auto Buffer() const ->Microsoft::WRL::ComPtr<ID3D12Resource> {
            return _indexBuffer;
        }

        auto StagingBuffer() const -> Microsoft::WRL::ComPtr<ID3D12Resource> {
            return _stagingBuffer;
        }

        auto Free() -> void {
            _stagingBuffer = nullptr;
        }

    private:
        Microsoft::WRL::ComPtr<ID3D12Resource> _stagingBuffer;
        Microsoft::WRL::ComPtr<ID3D12Resource> _indexBuffer;
        D3D12_INDEX_BUFFER_VIEW _indexBufferView;
    };
}
