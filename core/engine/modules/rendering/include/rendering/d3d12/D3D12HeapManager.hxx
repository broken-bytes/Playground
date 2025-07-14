#pragma once

#include <vector>

#include "rendering/d3d12/D3D12CPUResourceHandle.hxx"
#include "rendering/d3d12/D3D12GPUResourceHandle.hxx"
#include "rendering/d3d12/D3D12Heap.hxx"
#include <map>

namespace playground::rendering::d3d12
{
    enum class SRVHeapResource: uint16_t
    {
        FrameBuffer = 0, // 0 - 15 Frame buffers
        DepthBuffer = 32, // 16-31 Depth buffers
        Texture2D = 64, // 32-1023 Texture2D
        TextureCube = 1024, // 1024-1535 TextureCube
        ShadowMap = 1536, // 1536-2047 ShadowMap
        StructuredBuffer = 2048, // 2048-3071 StructuredBuffer
        ConstantBuffer = 3072, // 2048-4096 ConstantBuffer
        End = 4096, // End of SRV resources
    };

    enum class UAVHeapResource
    {
        StructuredBufferUAV = 0,
        AppendConsumeBuffer = 64,
        IndirectCommandBuffer = 128,
        RaytracingAccelerationStructure = 256,
        RaytracingShaderTable = 320,
        End = 384, // End of UAV resources
    };

    enum class RTVHeapResource
    {
        RenderTargetView = 0,
        SwapChainBuffer = 64,
        ShadowMapRTV = 128,
        End = 192, // End of RTV resources
    };

    enum class DSVHeapResource
    {
        DepthStencilView = 0,
        ShadowMapDSV = 128,
        End = 256, // End of DSV resources
    };

    enum class SamplerHeapResource
    {
        Sampler = 0,
        ComparisonSampler = 16,
        End = 32, // End of Sampler resources
    };

    class D3D12HeapManager
    {
    public:
        D3D12HeapManager(Microsoft::WRL::ComPtr<ID3D12Device> device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint16_t chunkSize);

        auto HandleAt(uint32_t index) -> std::shared_ptr<D3D12ResourceHandle>;
        auto NextHandle(SRVHeapResource resourceType) -> std::shared_ptr<D3D12ResourceHandle>;
        auto NextHandle(UAVHeapResource resourceType) -> std::shared_ptr<D3D12ResourceHandle>;
        auto NextHandle(RTVHeapResource resourceType) -> std::shared_ptr<D3D12ResourceHandle>;
        auto NextHandle(DSVHeapResource resourceType) -> std::shared_ptr<D3D12ResourceHandle>;
        auto NextHandle(SamplerHeapResource resourceType) -> std::shared_ptr<D3D12ResourceHandle>;
        auto Heap() -> std::shared_ptr<D3D12Heap> { return _heap; }

    private:
        Microsoft::WRL::ComPtr<ID3D12Device> _device;
        D3D12_DESCRIPTOR_HEAP_TYPE _type;
        std::shared_ptr<D3D12Heap> _heap;
        uint16_t _chunkSize;
        uint8_t _currentChunk = 0;
        std::map<SRVHeapResource, uint32_t> _srvResourceIndices;
        std::map<UAVHeapResource, uint32_t> _uavResourceIndices;
        std::map<RTVHeapResource, uint32_t> _rtvResourceIndices;
        std::map<DSVHeapResource, uint32_t> _dsvResourceIndices;
        std::map<SamplerHeapResource, uint32_t> _samplerResourceIndices;

        auto CreateHeap() -> std::shared_ptr<D3D12Heap>;
    };
}
