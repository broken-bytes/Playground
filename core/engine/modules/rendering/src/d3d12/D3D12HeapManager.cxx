#include "rendering/d3d12/D3D12HeapManager.hxx"
#include "rendering/d3d12/D3D12CPUResourceHandle.hxx"
#include "rendering/d3d12/D3D12GPUResourceHandle.hxx"

namespace playground::rendering::d3d12
{
    D3D12HeapManager::D3D12HeapManager(
        Microsoft::WRL::ComPtr<ID3D12Device> device,
        D3D12_DESCRIPTOR_HEAP_TYPE type,
        uint16_t chunkSize
    ) : _device(device), _type(type), _chunkSize(chunkSize)
    {
        _chunks = { CreateHeap() };
    }

    auto D3D12HeapManager::NextCpuHandle() -> std::shared_ptr<D3D12CPUResourceHandle>
    {
        for (auto& heap : _chunks)
        {
            if (heap->IsFilled())
            {
                continue;
            }

            return heap->NextCpuHandle();
        }

        // If we reach this we need to grow
        _chunks.emplace_back(CreateHeap());

        return _chunks.back()->NextCpuHandle();
    }

    auto D3D12HeapManager::NextGpuHandle() -> std::shared_ptr<D3D12GPUResourceHandle>
    {
        for (auto& heap : _chunks)
        {
            if (heap->IsFilled())
            {
                continue;
            }

            return heap->NextGpuHandle();
        }

        // If we reach this we need to grow
        _chunks.emplace_back(CreateHeap());

       return _chunks.back()->NextGpuHandle();
    }

    auto D3D12HeapManager::CreateHeap() -> std::shared_ptr<D3D12Heap>
    {
        return std::make_shared<D3D12Heap>(_device, _type, _chunkSize, _type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    }
}
