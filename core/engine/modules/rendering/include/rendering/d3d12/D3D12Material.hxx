#pragma once

#include <directx/d3dx12.h>
#include <wrl.h>
#include "rendering/Material.hxx"

namespace playground::rendering::d3d12 {
    class D3D12Material : public Material {
    public:
        Microsoft::WRL::ComPtr<ID3D12RootSignature> rootsignature;
        Microsoft::WRL::ComPtr<ID3D12PipelineState> pso;

        D3D12Material(
            Microsoft::WRL::ComPtr<ID3D12RootSignature> rootsignature,
            Microsoft::WRL::ComPtr<ID3D12PipelineState> pso,
            std::map<std::string, uint64_t> textures,
            std::map<std::string, float> floats,
            std::map<std::string, uint32_t> ints,
            std::map<std::string, bool> bools,
            std::map<std::string, std::array<float, 2>> vec2s,
            std::map<std::string, std::array<float, 3>> vec3s,
            std::map<std::string, std::array<float, 4>> vec4s) : Material(textures, floats, ints, bools, vec2s, vec3s, vec4s)
        {

        }
    };
}
