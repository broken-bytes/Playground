#pragma once

#include <directx/d3dx12.h>
#include "rendering/CPUResourceHandle.hxx"

namespace playground::rendering::d3d12 {
    class D3D12ResourceHandle : public CPUResourceHandle
    {
    public:
        D3D12ResourceHandle(uint32_t id, CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle) : CPUResourceHandle(id)
        {
            _cpuHandle = cpuHandle;
            _gpuHandle = gpuHandle;
        }

        D3D12ResourceHandle(uint32_t id, CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle) : CPUResourceHandle(id) {
            _cpuHandle = cpuHandle;
            _gpuHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE();
        }

        ~D3D12ResourceHandle() final
        {

        }

        auto GetCPUHandle() const -> CD3DX12_CPU_DESCRIPTOR_HANDLE
        {
            return _cpuHandle;
        }

        auto GetGPUHandle() const -> CD3DX12_GPU_DESCRIPTOR_HANDLE
        {
            return _gpuHandle;
        }

    private:
        CD3DX12_CPU_DESCRIPTOR_HANDLE _cpuHandle;
        CD3DX12_GPU_DESCRIPTOR_HANDLE _gpuHandle;
    };
}
