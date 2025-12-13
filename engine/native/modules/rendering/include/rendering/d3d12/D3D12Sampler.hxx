#pragma once

#include <wrl.h>
#include <directx/d3dx12.h>
#include "rendering/Sampler.hxx"

namespace playground::rendering::d3d12 {
    inline auto TranslateTextureFiltering(TextureFiltering filtering) -> D3D12_FILTER {
        switch (filtering) {
        case TextureFiltering::Point:
            return D3D12_FILTER_MIN_MAG_MIP_POINT;
        case TextureFiltering::Bilinear:
            return D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
        case TextureFiltering::Trilinear:
            return D3D12_FILTER_MIN_MAG_MIP_LINEAR;
        case TextureFiltering::Anisotropic:
            return D3D12_FILTER_ANISOTROPIC;
        default:
            return D3D12_FILTER_MIN_MAG_MIP_POINT;
        }
    }

    inline auto TranslateTextureWrapping(TextureWrapping wrapping) -> D3D12_TEXTURE_ADDRESS_MODE {
        switch (wrapping) {
        case TextureWrapping::Repeat:
            return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        case TextureWrapping::Clamp:
            return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        case TextureWrapping::Mirror:
            return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
        default:
            return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        }
    }

    class D3D12TextureSampler : public Sampler {
    public:
        D3D12TextureSampler(
            Microsoft::WRL::ComPtr<ID3D12Device9> device,
            std::shared_ptr<D3D12ResourceHandle> handle,
            TextureFiltering filtering,
            TextureWrapping wrapping
        ) {
            _handle = handle;
            D3D12_SAMPLER_DESC samplerDesc = {};
            samplerDesc.Filter = TranslateTextureFiltering(filtering);
            samplerDesc.AddressU = TranslateTextureWrapping(wrapping);
            samplerDesc.AddressV = TranslateTextureWrapping(wrapping);
            samplerDesc.AddressW = TranslateTextureWrapping(wrapping);
            samplerDesc.MipLODBias = 0.0f;
            samplerDesc.MaxAnisotropy = 16;
            samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
            samplerDesc.BorderColor[0] = samplerDesc.BorderColor[1] = samplerDesc.BorderColor[2] = samplerDesc.BorderColor[3] = 1.0f;
            samplerDesc.MinLOD = 0;
            samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;

            device->CreateSampler(&samplerDesc, _handle->GetCPUHandle());
        }

        auto CPUHandle() -> CD3DX12_CPU_DESCRIPTOR_HANDLE {
            return _handle->GetCPUHandle();
        }

        auto GPUHandle() -> CD3DX12_GPU_DESCRIPTOR_HANDLE {
            return _handle->GetGPUHandle();
        }

    private:
        std::shared_ptr<D3D12ResourceHandle> _handle;
    };
}
