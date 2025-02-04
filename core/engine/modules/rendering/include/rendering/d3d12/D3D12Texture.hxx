#pragma once

#include <cstdint>
#include <stdexcept>
#include <wrl.h>
#include <directx/d3dx12.h>
#include "rendering/Texture.hxx"
#include "rendering/d3d12/D3D12CPUResourceHandle.hxx"

namespace playground::rendering::d3d12 {
    class D3D12Texture : public Texture {
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
        D3D12Texture(
            Microsoft::WRL::ComPtr<ID3D12Device9> device,
            uint32_t width,
            uint32_t height,
            void* data,
            std::shared_ptr<D3D12ResourceHandle> handle
        ) {
            D3D12_RESOURCE_DESC textureDesc = {};
            textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
            textureDesc.Width = width;
            textureDesc.Height = height;
            textureDesc.DepthOrArraySize = 1;
            textureDesc.MipLevels = 1;
            textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
            textureDesc.SampleDesc.Count = 1;
            textureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
            textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

            D3D12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
            device->CreateCommittedResource(
                &heapProps,
                D3D12_HEAP_FLAG_NONE,
                &textureDesc,
                D3D12_RESOURCE_STATE_COPY_DEST,  // Start in COPY_DEST state
                nullptr,
                IID_PPV_ARGS(&_texture)
            );

            const UINT64 uploadBufferSize = GetRequiredIntermediateSize(_texture.Get(), 0, 1);

            D3D12_HEAP_PROPERTIES uploadHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
            D3D12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);

            device->CreateCommittedResource(
                &uploadHeapProps,
                D3D12_HEAP_FLAG_NONE,
                &bufferDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&_stagingBuffer)
            );

            _data = new uint8_t[width * height * 4];
            std::memcpy(_data, data, width * height * 4);
            _textureData.pData = _data;
            _textureData.RowPitch = width * 4;
            _textureData.SlicePitch = _textureData.RowPitch * height;

            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MostDetailedMip = 0;
            srvDesc.Texture2D.MipLevels = 1;
            srvDesc.Texture2D.PlaneSlice = 0;
            srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

            _handle = handle;

            device->CreateShaderResourceView(_texture.Get(), &srvDesc, handle->GetCPUHandle());
        }

        auto Texture() const -> Microsoft::WRL::ComPtr<ID3D12Resource> {
            return _texture;
        }

        auto StagingBuffer() const -> Microsoft::WRL::ComPtr<ID3D12Resource> {
            return _stagingBuffer;
        }

        auto TextureData() const -> const D3D12_SUBRESOURCE_DATA& {
            return _textureData;
        }

        auto Free() {
            delete[] _data;
            _stagingBuffer = nullptr;
        }

        auto CPUHandle() -> CD3DX12_CPU_DESCRIPTOR_HANDLE {
            return _handle->GetCPUHandle();
        }

        auto GPUHandle() -> CD3DX12_GPU_DESCRIPTOR_HANDLE {
            return _handle->GetGPUHandle();
        }

    private:
        void* _data;
        Microsoft::WRL::ComPtr<ID3D12Resource> _texture;
        Microsoft::WRL::ComPtr<ID3D12Resource> _stagingBuffer;
        D3D12_SUBRESOURCE_DATA _textureData;
        std::shared_ptr<D3D12ResourceHandle> _handle;
    };
}
