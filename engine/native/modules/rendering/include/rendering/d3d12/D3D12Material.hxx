#pragma once

#include <directx/d3dx12.h>
#include <wrl.h>
#include "rendering/Material.hxx"

namespace playground::rendering::d3d12 {
    class D3D12Material : public Material {
    public:
        Microsoft::WRL::ComPtr<ID3D12PipelineState> pso;

        D3D12Material(
            Microsoft::WRL::ComPtr<ID3D12PipelineState> pso) : Material(), pso(pso)
        {

        }
    };
}
