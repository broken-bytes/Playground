#pragma once

#include "rendering/d3d12/D3D12Device.hxx"
#include "rendering/d3d12/D3D12DepthBuffer.hxx"
#include "rendering/d3d12/D3D12CPUResourceHandle.hxx"
#include "rendering/ShadowMap.hxx"
#include <d3d12.h>
#include <wrl.h>
#include <cstdint>
#include <memory>

namespace playground::rendering::d3d12 {
    class D3D12ShadowMap : public ShadowMap {
    public:
        D3D12ShadowMap(
            Microsoft::WRL::ComPtr<ID3D12Device9> device,
            std::shared_ptr<D3D12ResourceHandle>& depthHandle,
            std::shared_ptr<D3D12ResourceHandle>& srvHandle,
            uint16_t width,
            uint16_t height,
            std::string name
        ) : _dsvHandle(depthHandle), _srvHandle(srvHandle), ShadowMap(width, height) {
            D3D12_RESOURCE_DESC desc = {};
            desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
            desc.Alignment = 0;
            desc.Width = width;
            desc.Height = height;
            desc.DepthOrArraySize = 1;
            desc.MipLevels = 1;
            desc.Format = DXGI_FORMAT_R32_TYPELESS;
            desc.SampleDesc.Count = 1;
            desc.SampleDesc.Quality = 0;
            desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
            desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

            D3D12_CLEAR_VALUE clearValue = {};
            clearValue.Format = DXGI_FORMAT_D32_FLOAT;
            clearValue.DepthStencil.Depth = 1.0f;
            clearValue.DepthStencil.Stencil = 0;

            D3D12_HEAP_PROPERTIES heapProps = {};
            heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
            device->CreateCommittedResource(
                &heapProps,
                D3D12_HEAP_FLAG_NONE,
                &desc,
                D3D12_RESOURCE_STATE_DEPTH_WRITE,
                &clearValue,
                IID_PPV_ARGS(&_shadowMap)
            );

            D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
            dsvDesc.Format = DXGI_FORMAT_D32_FLOAT; // For depth-only rendering
            dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
            dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

            device->CreateDepthStencilView(_shadowMap.Get(), &dsvDesc, _dsvHandle->GetCPUHandle());

            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Format = DXGI_FORMAT_R32_FLOAT; // Shader-readable view of depth
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srvDesc.Texture2D.MipLevels = 1;

            _shadowMap->SetName(std::wstring(name.begin(), name.end()).c_str());

            device->CreateShaderResourceView(_shadowMap.Get(), &srvDesc, _srvHandle->GetCPUHandle());
        }

        std::shared_ptr<DepthBuffer> GetDepthBuffer() override {
            return std::make_shared<D3D12DepthBuffer>(_shadowMap, _dsvHandle->GetCPUHandle());
        }

        uint32_t ID() const override {
            return _srvHandle->ID();
        }

        Microsoft::WRL::ComPtr<ID3D12Resource> Resource() const {
            return _shadowMap;
        }

    private:
        Microsoft::WRL::ComPtr<ID3D12Resource> _shadowMap;
        std::shared_ptr<D3D12ResourceHandle> _dsvHandle;
        std::shared_ptr<D3D12ResourceHandle> _srvHandle;
    };
}
