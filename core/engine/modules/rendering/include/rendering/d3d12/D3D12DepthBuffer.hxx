#pragma once

#include <directx/d3dx12.h>
#include <wrl.h>

#include "D3D12CPUResourceHandle.hxx"
#include "rendering/RenderTarget.hxx"

namespace playground::rendering::d3d12
{
    class D3D12DepthBuffer : public rendering::DepthBuffer
    {
    public:
        D3D12DepthBuffer(Microsoft::WRL::ComPtr<ID3D12Resource> resource, D3D12_CPU_DESCRIPTOR_HANDLE handle)
        {
            _resource = resource;
            _handle = handle;
        }

        ~D3D12DepthBuffer() {
        }

        Microsoft::WRL::ComPtr<ID3D12Resource> Resource() const
        {
            return _resource;
        }

        D3D12_CPU_DESCRIPTOR_HANDLE Handle() const
        {
            return _handle;
        }

    private:
        Microsoft::WRL::ComPtr<ID3D12Resource> _resource;
        D3D12_CPU_DESCRIPTOR_HANDLE _handle;
    };
}
