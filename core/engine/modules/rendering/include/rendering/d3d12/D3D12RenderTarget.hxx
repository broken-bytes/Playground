#pragma once

#include <wrl.h>
#include <directx/d3dx12.h>

#include "D3D12CPUResourceHandle.hxx"
#include "rendering/RenderTarget.hxx"

namespace playground::rendering::d3d12
{
    class D3D12RenderTarget : public rendering::RenderTarget
    {
    public:
        D3D12RenderTarget(
            Microsoft::WRL::ComPtr<ID3D12Resource> resource,
            D3D12_CPU_DESCRIPTOR_HANDLE handle,
            std::shared_ptr<D3D12ResourceHandle> srvHandle
        )
        {
            _resource = resource;
            _handle = handle;
            _srvHandle = srvHandle;
        }

        ~D3D12RenderTarget() {
        }

        Microsoft::WRL::ComPtr<ID3D12Resource> Resource() const
        {
            return _resource;
        }

        D3D12_CPU_DESCRIPTOR_HANDLE Handle() const
        {
            return _handle;
        }

        std::shared_ptr<D3D12ResourceHandle> SRVHandle() const
        {
            return _srvHandle;
        }

    private:
        Microsoft::WRL::ComPtr<ID3D12Resource> _resource;
        D3D12_CPU_DESCRIPTOR_HANDLE _handle;
        std::shared_ptr<D3D12ResourceHandle> _srvHandle;
    };
}
