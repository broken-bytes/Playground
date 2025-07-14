#pragma once

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <wrl.h>
#include <directx/d3dx12.h>
#include "rendering/StructuredBuffer.hxx"
#include "rendering/d3d12/D3D12HeapManager.hxx"

namespace playground::rendering::d3d12 {
    class D3D12StructuredBuffer : public StructuredBuffer {
    public:
        D3D12StructuredBuffer(
            Microsoft::WRL::ComPtr<ID3D12Device9> device,
            CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle,
            CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle,
            Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap,
            void* data,
            size_t count,
            size_t stride,
            std::string name
        ) {
            _heap = heap;
            const UINT bufferSize = count * stride;

            _cpuHandle = cpuHandle;
            _gpuHandle = gpuHandle;

            D3D12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
            D3D12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize, D3D12_RESOURCE_FLAG_NONE);

            if (FAILED(device->CreateCommittedResource(
                &heapProps,
                D3D12_HEAP_FLAG_NONE,
                &bufferDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&_buffer))))
            {
                throw std::runtime_error("Failed to create constant buffer");
            }


            UINT descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
            srvDesc.Buffer.FirstElement = 0;
            srvDesc.Buffer.NumElements = count;
            srvDesc.Buffer.StructureByteStride = stride;
            srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
            srvDesc.Format = DXGI_FORMAT_UNKNOWN; // Structured buffers have no format
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

            device->CreateShaderResourceView(_buffer.Get(), &srvDesc, _cpuHandle);

            _data = std::malloc(bufferSize);

            _buffer->Map(0, nullptr, reinterpret_cast<void**>(&_data));

            _buffer->SetName(std::wstring(name.begin(), name.end()).c_str());

            _alignedStride = stride;
        }

        virtual ~D3D12StructuredBuffer() {
            _buffer->Unmap(0, nullptr);
        }

        inline void SetData(const void* data, size_t count, size_t offset) override {
            std::memcpy(static_cast<uint8_t*>(_data) + offset * _alignedStride, data, count * _alignedStride);
        }

        [[nodiscard]]
        auto Buffer() -> Microsoft::WRL::ComPtr<ID3D12Resource> { return _buffer; }

        [[nodiscard]]
        auto View() -> D3D12_SHADER_RESOURCE_VIEW_DESC { return _view; }

        [[nodiscard]]
        auto CPUHandle() -> CD3DX12_CPU_DESCRIPTOR_HANDLE { return _cpuHandle; }

        [[nodiscard]]
        auto GPUHandle() -> CD3DX12_GPU_DESCRIPTOR_HANDLE { return _gpuHandle; }

    private:
        Microsoft::WRL::ComPtr<ID3D12Resource> _buffer;
        D3D12_SHADER_RESOURCE_VIEW_DESC _view;
        CD3DX12_CPU_DESCRIPTOR_HANDLE _cpuHandle;
        CD3DX12_GPU_DESCRIPTOR_HANDLE _gpuHandle;
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _heap;
        uint64_t _alignedStride;
        void* _data;
    };
}
