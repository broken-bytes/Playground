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
            _srvResourceIndices[SRVHeapResource::FrameBuffer] = 0;
            _srvResourceIndices[SRVHeapResource::DepthBuffer] = 0;
            _srvResourceIndices[SRVHeapResource::Texture2D] = 0;
            _srvResourceIndices[SRVHeapResource::TextureCube] = 0;
            _srvResourceIndices[SRVHeapResource::ShadowMap] = 0;
            _srvResourceIndices[SRVHeapResource::StructuredBuffer] = 0;
            break;
        case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
            _heap->Native()->SetName(L"SAMPLER Heap");
            _samplerResourceIndices[SamplerHeapResource::Sampler] = 0;
            _samplerResourceIndices[SamplerHeapResource::ComparisonSampler] = 0;
            break;
        case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
            _heap->Native()->SetName(L"RTV Heap");
            _rtvResourceIndices[RTVHeapResource::RenderTargetView] = 0;
            _rtvResourceIndices[RTVHeapResource::SwapChainBuffer] = 0;
            _rtvResourceIndices[RTVHeapResource::ShadowMapRTV] = 0;
            break;
        case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
            _heap->Native()->SetName(L"DSV Heap");
            _dsvResourceIndices[DSVHeapResource::DepthStencilView] = 0;
            _dsvResourceIndices[DSVHeapResource::ShadowMapDSV] = 0;
            break;
        case D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES:
            _heap->Native()->SetName(L"NUM_TYPES Heap");
            // This is a placeholder, we should never reach this case
            break;
        default:
            break;
        }
    }

    auto D3D12HeapManager::HandleAt(uint32_t index) -> std::shared_ptr<D3D12ResourceHandle> {
        return _heap->HandleFor(index, index);
    }

    auto D3D12HeapManager::NextHandle(SRVHeapResource resourceType) -> std::shared_ptr<D3D12ResourceHandle> {
        auto index = _srvResourceIndices[resourceType];
        auto handle = _heap->HandleFor((uint32_t)resourceType + index, index);

        _srvResourceIndices[resourceType]++;

        return handle;
    }

    auto D3D12HeapManager::NextHandle(UAVHeapResource resourceType) -> std::shared_ptr<D3D12ResourceHandle> {
        auto index = _uavResourceIndices[resourceType];
        auto handle = _heap->HandleFor((uint32_t)resourceType + index, index);

        _uavResourceIndices[resourceType]++;

        return handle;
    }

    auto D3D12HeapManager::NextHandle(RTVHeapResource resourceType) -> std::shared_ptr<D3D12ResourceHandle> {
        auto index = _rtvResourceIndices[resourceType];
        auto handle = _heap->HandleFor((uint32_t)resourceType + index, index);

        _rtvResourceIndices[resourceType]++;

        return handle;;
    }

    auto D3D12HeapManager::NextHandle(DSVHeapResource resourceType) -> std::shared_ptr<D3D12ResourceHandle> {
        auto index = _dsvResourceIndices[resourceType];
        auto handle = _heap->HandleFor((uint32_t)resourceType + index, index);

        _dsvResourceIndices[resourceType]++;

        return handle;
    }

    auto D3D12HeapManager::NextHandle(SamplerHeapResource resourceType) -> std::shared_ptr<D3D12ResourceHandle> {
        auto index = _samplerResourceIndices[resourceType];
        auto handle = _heap->HandleFor((uint32_t)resourceType + index, index);

        _samplerResourceIndices[resourceType]++;

        return handle;
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
