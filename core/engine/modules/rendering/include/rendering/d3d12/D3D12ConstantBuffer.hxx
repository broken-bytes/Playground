#pragma once

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <wrl.h>
#include <directx/d3dx12.h>
#include "rendering/ConstantBuffer.hxx"
#include "rendering/d3d12/D3D12HeapManager.hxx"

namespace playground::rendering::d3d12 {
    class D3D12ConstantBuffer : public ConstantBuffer {
    public:
        D3D12ConstantBuffer(
            Microsoft::WRL::ComPtr<ID3D12Device9> device,
            CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle,
            CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle,
            Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap,
            const void* data,
            uint64_t count,
            uint64_t alignedStride,
            BindingMode bindingMode,
            std::string name
        ) {
            _heap = heap;
            const UINT bufferSize = (count * alignedStride + 255) & ~255;

            assert(bufferSize > 0);

            D3D12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
            D3D12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

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

            UINT viewCount = count;

            _cpuHandles.resize(viewCount);
            _gpuHandles.resize(viewCount);

            UINT descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

            if (bindingMode == BindingMode::DescriptorTable) {
                for (UINT i = 0; i < viewCount; i++) {
                    D3D12_CONSTANT_BUFFER_VIEW_DESC viewDesc = {};
                    viewDesc.BufferLocation = _buffer->GetGPUVirtualAddress() + i * alignedStride;
                    viewDesc.SizeInBytes = alignedStride;

                    _gpuHandles[i] = gpuHandle.Offset(i, descriptorSize);
                    _cpuHandles[i] = cpuHandle.Offset(i, descriptorSize);

                    device->CreateConstantBufferView(&viewDesc, _cpuHandles[i]);
                }
            }
            else {
                D3D12_CONSTANT_BUFFER_VIEW_DESC viewDesc = {};
                viewDesc.BufferLocation = _buffer->GetGPUVirtualAddress();
                viewDesc.SizeInBytes = alignedStride;

                _gpuHandles[0] = gpuHandle.Offset(0, descriptorSize);
                _cpuHandles[0] = cpuHandle.Offset(0, descriptorSize);

                _view = viewDesc;

                device->CreateConstantBufferView(&viewDesc, _cpuHandles[0]);
            }

            _buffer->Map(0, nullptr, reinterpret_cast<void**>(&_data));

            _buffer->SetName(std::wstring(name.begin(), name.end()).c_str());

            _alignedStride = alignedStride;
        }

        virtual ~D3D12ConstantBuffer() {
            _buffer->Unmap(0, nullptr);
        }

        inline void SetData(const void* data, size_t count, size_t offset) override {
            /*
            size_t byteOffset = offset * _alignedStride;
            size_t byteSize = count * _alignedStride;

            std::memcpy(static_cast<uint8_t*>(_mappedData) + byteOffset, data, byteSize);
            */
            std::memcpy(static_cast<uint8_t*>(_data) + offset * _alignedStride, data, count * _alignedStride);
        }

        [[nodiscard]]
        auto Buffer() -> Microsoft::WRL::ComPtr<ID3D12Resource> { return _buffer; }

        [[nodiscard]]
        auto CPUHandle(uint32_t index) -> CD3DX12_CPU_DESCRIPTOR_HANDLE { return _cpuHandles[index]; }

        [[nodiscard]]
        auto GPUHandle(uint32_t index) -> CD3DX12_GPU_DESCRIPTOR_HANDLE { return _gpuHandles[index]; }

        [[nodiscard]]
        auto GPUAdress(uint32_t index) -> D3D12_GPU_VIRTUAL_ADDRESS {
            return _buffer->GetGPUVirtualAddress() + index * _alignedStride;
        }

    private:
        Microsoft::WRL::ComPtr<ID3D12Resource> _buffer;
        D3D12_CONSTANT_BUFFER_VIEW_DESC _view;
        std::vector<CD3DX12_CPU_DESCRIPTOR_HANDLE> _cpuHandles;
        std::vector<CD3DX12_GPU_DESCRIPTOR_HANDLE> _gpuHandles;
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _heap;
        uint64_t _alignedStride;
        void* _data;
    };
}
