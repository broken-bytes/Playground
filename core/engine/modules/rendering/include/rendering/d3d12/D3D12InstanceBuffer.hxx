#pragma once

#include <cstdint>
#include <stdexcept>
#include <wrl.h>
#include <directx/d3dx12.h>
#include "rendering/InstanceBuffer.hxx"
#include "rendering/Vertex.hxx"

namespace playground::rendering::d3d12 {
    class D3D12InstanceBuffer : public rendering::InstanceBuffer {
    public:

        auto CreateStagingBuffer(
            size_t count,
            size_t alignedStride,
            Microsoft::WRL::ComPtr<ID3D12Device9> device,
            D3D12_RESOURCE_DESC bufferDesc,
            Microsoft::WRL::ComPtr<ID3D12Resource>& buffer
        ) {
            const UINT bufferSize = (count * alignedStride + 255) & ~255;
            // Create a default heap resource for the vertex buffer
            D3D12_HEAP_PROPERTIES heapProps = {};
            heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

            _alignedStride = alignedStride;

            HRESULT hr = device->CreateCommittedResource(
                &heapProps,
                D3D12_HEAP_FLAG_NONE,
                &bufferDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&buffer)
            );
            if (FAILED(hr)) {
                throw std::runtime_error("Failed to create instance buffer resource.");
            }

            _mappedData = nullptr;
            D3D12_RANGE readRange = {};
            buffer->Map(0, &readRange, &_mappedData);
        }

        auto PrepareStaticBuffer(
            Microsoft::WRL::ComPtr<ID3D12Device9> device,
            D3D12_RESOURCE_DESC bufferDesc,
            Microsoft::WRL::ComPtr<ID3D12Resource>& buffer
        ) {
            // Create a default heap resource for the instance buffer
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
                throw std::runtime_error("Failed to create instance buffer resource.");
            }
        }

        D3D12InstanceBuffer(
            Microsoft::WRL::ComPtr<ID3D12Device9> device,
            size_t count,
            size_t alignedStride
        ) : rendering::InstanceBuffer(count) {

            const UINT bufferSize = (count * alignedStride + 255) & ~255;

            D3D12_RESOURCE_DESC bufferDesc = {};
            bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
            bufferDesc.Width = bufferSize;
            bufferDesc.Height = 1;
            bufferDesc.DepthOrArraySize = 1;
            bufferDesc.MipLevels = 1;
            bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
            bufferDesc.SampleDesc.Count = 1;
            bufferDesc.SampleDesc.Quality = 0;
            bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
            bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

            CreateStagingBuffer(count,alignedStride,  device, bufferDesc, _stagingBuffer);
            PrepareStaticBuffer(device, bufferDesc, _vertexBuffer);

            _instanceBufferView.BufferLocation = _vertexBuffer->GetGPUVirtualAddress();
            _instanceBufferView.StrideInBytes = alignedStride;
            _instanceBufferView.SizeInBytes = bufferSize;
        };

        ~D3D12InstanceBuffer() {
            _stagingBuffer->Unmap(0, nullptr);
            _mappedData = nullptr;
        }

        auto Id() const -> uint64_t override {
            return 0;
        }

        
        inline void SetData(const void* data, size_t count, size_t offset) override {
            /*
            size_t byteOffset = offset * _alignedStride;
            size_t byteSize = count * _alignedStride;

            std::memcpy(static_cast<uint8_t*>(_mappedData) + byteOffset, data, byteSize);
            */
            std::memcpy(static_cast<uint8_t*>(_mappedData) + offset * _alignedStride, data, count * _alignedStride);
        }

        void Bind() const override {
        }

        auto View() const -> const D3D12_VERTEX_BUFFER_VIEW& {
            return _instanceBufferView;
        }

        auto Buffer() const -> Microsoft::WRL::ComPtr<ID3D12Resource> {
            return _vertexBuffer;
        }

        auto StagingBuffer() const -> Microsoft::WRL::ComPtr<ID3D12Resource> {
            return _stagingBuffer;
        }

        auto Free() -> void {
            _stagingBuffer = nullptr;
        }

    private:
        Microsoft::WRL::ComPtr<ID3D12Resource> _stagingBuffer;
        Microsoft::WRL::ComPtr<ID3D12Resource> _vertexBuffer;
        D3D12_VERTEX_BUFFER_VIEW _instanceBufferView;
        void* _mappedData = nullptr;
        size_t _alignedStride;
    };
}
