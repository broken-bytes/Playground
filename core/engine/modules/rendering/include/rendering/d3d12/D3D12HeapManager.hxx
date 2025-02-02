#pragma once

#include <vector>

#include "rendering/d3d12/D3D12CPUResourceHandle.hxx"
#include "rendering/d3d12/D3D12GPUResourceHandle.hxx"
#include "rendering/d3d12/D3D12Heap.hxx"

namespace playground::rendering::d3d12
{
    class D3D12HeapManager
    {
    public:
        D3D12HeapManager(Microsoft::WRL::ComPtr<ID3D12Device> device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint16_t chunkSize);
        auto NextCpuHandle() -> std::shared_ptr<D3D12CPUResourceHandle>;
        auto NextGpuHandle() -> std::shared_ptr<D3D12GPUResourceHandle>;

    private:
        Microsoft::WRL::ComPtr<ID3D12Device> _device;
        D3D12_DESCRIPTOR_HEAP_TYPE _type;
        std::vector<std::shared_ptr<D3D12Heap>> _chunks;
        uint16_t _chunkSize;

        auto CreateHeap() -> std::shared_ptr<D3D12Heap>;
    };
}
