#pragma once

#include <directx/d3dx12.h>
#include <wrl.h>
#include <memory>
#include "D3D12CPUResourceHandle.hxx"
#include "rendering/RenderTarget.hxx"

namespace playground::rendering::d3d12
{
    class D3D12DepthBuffer : public rendering::DepthBuffer
    {
    public:
        D3D12DepthBuffer
        (Microsoft::WRL::ComPtr<ID3D12Resource> resource,
            D3D12_CPU_DESCRIPTOR_HANDLE handle,
            D3D12_CPU_DESCRIPTOR_HANDLE readOnlyHandle,
            std::shared_ptr<D3D12ResourceHandle> srvHandle
        )
        {
            _resource = resource;
            _handle = handle;
            _readOnlyHandle = readOnlyHandle;
            _srvHandle = srvHandle;
        }

        D3D12DepthBuffer
        (Microsoft::WRL::ComPtr<ID3D12Resource> resource,
            D3D12_CPU_DESCRIPTOR_HANDLE handle,
            std::shared_ptr<D3D12ResourceHandle> srvHandle
        )
        {
            _resource = resource;
            _handle = handle;
            _readOnlyHandle = D3D12_CPU_DESCRIPTOR_HANDLE();
            _srvHandle = srvHandle;
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

        D3D12_CPU_DESCRIPTOR_HANDLE ReadOnlyHandle() const
        {
            return _readOnlyHandle;
        }

        std::shared_ptr<D3D12ResourceHandle> SRVHandle() const
        {
            return _srvHandle;
        }

        void SetState(D3D12_RESOURCE_STATES state)
        {
            _state = state;
        }

        D3D12_RESOURCE_STATES GetState() const
        {
            return _state;
        }

    private:
        Microsoft::WRL::ComPtr<ID3D12Resource> _resource;
        D3D12_CPU_DESCRIPTOR_HANDLE _handle;
        D3D12_CPU_DESCRIPTOR_HANDLE _readOnlyHandle;
        std::shared_ptr<D3D12ResourceHandle> _srvHandle;
        D3D12_RESOURCE_STATES _state = D3D12_RESOURCE_STATE_DEPTH_WRITE;
    };
}
