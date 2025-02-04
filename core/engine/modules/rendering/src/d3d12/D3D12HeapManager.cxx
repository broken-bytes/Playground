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
        _heap = CreateHeap();

        switch (type)
        {
        case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
            _heap->Native()->SetName(L"CBV_SRV_UAV Heap");
            break;
        case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
            _heap->Native()->SetName(L"SAMPLER Heap");
            break;
        case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
            _heap->Native()->SetName(L"RTV Heap");
            break;
        case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
            _heap->Native()->SetName(L"DSV Heap");
            break;
        case D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES:
            _heap->Native()->SetName(L"NUM_TYPES Heap");
            break;
        default:
            break;
        }
    }

    auto D3D12HeapManager::HandleAt(uint32_t index) -> std::shared_ptr<D3D12ResourceHandle> {
        return _heap->HandleFor(index);
    }

    auto D3D12HeapManager::NextHandle() -> std::shared_ptr<D3D12ResourceHandle>
    {
        return _heap->NextHandle();
    }

    auto D3D12HeapManager::CreateHeap() -> std::shared_ptr<D3D12Heap>
    {
        _currentChunk++;
        return std::make_shared<D3D12Heap>(
            _device,
            _type,
            _chunkSize * _currentChunk,
            _type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV || _type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER
        );
    }
}
