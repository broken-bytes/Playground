#pragma once

#include <directx/d3dx12.h>
#include "rendering/GPUResourceHandle.hxx"

namespace playground::rendering::d3d12 {
    class D3D12GPUResourceHandle : public GPUResourceHandle
    {
    public:
        explicit D3D12GPUResourceHandle(CD3DX12_GPU_DESCRIPTOR_HANDLE handle)
        {
            _handle = handle;
        }

        ~D3D12GPUResourceHandle() final
        {

        }

        CD3DX12_GPU_DESCRIPTOR_HANDLE GetHandle() const
        {
            return _handle;
        }

    private:
        CD3DX12_GPU_DESCRIPTOR_HANDLE _handle;
    };
}
