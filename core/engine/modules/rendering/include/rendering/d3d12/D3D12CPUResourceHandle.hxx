#pragma once

#include <directx/d3dx12.h>
#include "rendering/CPUResourceHandle.hxx"

namespace playground::rendering::d3d12 {
    class D3D12CPUResourceHandle : public CPUResourceHandle
    {
    public:
        D3D12CPUResourceHandle(CD3DX12_CPU_DESCRIPTOR_HANDLE handle)
        {
            _handle = handle;
        }

        ~D3D12CPUResourceHandle() final
        {

        }

        auto GetHandle() const -> CD3DX12_CPU_DESCRIPTOR_HANDLE
        {
            return _handle;
        }

    private:
        CD3DX12_CPU_DESCRIPTOR_HANDLE _handle;
    };
}
