#pragma once

#include <cstdint>
#include <stdexcept>
#include <wrl.h>
#include <directx/d3dx12.h>
#include "rendering/RootSignature.hxx"

namespace playground::rendering::d3d12 {
    class D3D12RootSignature : public rendering::RootSignature {
    public:
        D3D12RootSignature(Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature) {
            _rootSignature = rootSignature;
        }

        ~D3D12RootSignature() {

        }

        auto GetRootSignature() -> Microsoft::WRL::ComPtr<ID3D12RootSignature> {
            return _rootSignature;
        }

    private:
        Microsoft::WRL::ComPtr<ID3D12RootSignature> _rootSignature;
    };
}
