#pragma once

#include <stdexcept>
#include <wrl.h>
#include <directx/d3d12.h>
#include <directx/d3dx12.h>
#include <directx-dxc/dxcapi.h>
#include "rendering/PipelineState.hxx"

namespace playground::rendering::d3d12 {
    inline Microsoft::WRL::ComPtr<IDxcBlob> CompileShaderWithDxc(
        const std::string& shaderCode, const std::wstring& entryPoint, const std::wstring& target)
    {
        Microsoft::WRL::ComPtr<IDxcUtils> utils;
        Microsoft::WRL::ComPtr<IDxcCompiler3> compiler;
        Microsoft::WRL::ComPtr<IDxcBlobEncoding> sourceBlob;
        Microsoft::WRL::ComPtr<IDxcResult> result;

        // Initialize DXC Compiler and Utility
        DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils));
        DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler));

        // Create a blob from the shader source string
        utils->CreateBlob(shaderCode.c_str(), (uint32_t)shaderCode.size(), DXC_CP_UTF8, &sourceBlob);

        // Create a DxcBuffer to pass to Compile()
        DxcBuffer sourceBuffer;
        sourceBuffer.Ptr = sourceBlob->GetBufferPointer();
        sourceBuffer.Size = sourceBlob->GetBufferSize();
        sourceBuffer.Encoding = DXC_CP_UTF8;

        // Define compiler arguments
        const wchar_t* arguments[] = {
            L"-E", entryPoint.c_str(),
            L"-T", target.c_str(),
            L"-O3",
            L"-Zi"
        };

        // Compile the shader
        HRESULT hr = compiler->Compile(
            &sourceBuffer, // Use DxcBuffer instead of IDxcBlob*
            arguments, _countof(arguments),
            nullptr, // No include handler
            IID_PPV_ARGS(&result)
        );

        if (FAILED(hr)) {
            throw std::runtime_error("DXC Compilation failed.");
        }

        // Retrieve compiled shader blob
        Microsoft::WRL::ComPtr<IDxcBlobUtf8> errors;
        if (FAILED(result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr))) {
            throw std::runtime_error("Failed to retrieve error messages.");
        }
        if (errors && errors->GetStringLength() > 0) {
            OutputDebugStringA(errors->GetStringPointer()); // Print error messages
        }

        Microsoft::WRL::ComPtr<IDxcBlob> shaderBlob;
        result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
        return shaderBlob;
    }

    class D3D12PipelineState : public PipelineState {
    public:
        D3D12PipelineState(
            Microsoft::WRL::ComPtr<ID3D12Device9> device,
            Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature,
            const std::string& vertexShader,
            const std::string& pixelShader
        ) {
            D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
            ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

            psoDesc.pRootSignature = rootSignature.Get();

            auto vertexShaderBlob = CompileShaderWithDxc(vertexShader, L"VSMain", L"vs_6_0");
            auto pixelShaderBlob = CompileShaderWithDxc(pixelShader, L"PSMain", L"ps_6_0");

            D3D12_SHADER_BYTECODE vertexShaderBytecode = {};
            vertexShaderBytecode.pShaderBytecode = vertexShaderBlob.Get()->GetBufferPointer();
            vertexShaderBytecode.BytecodeLength = vertexShaderBlob.Get()->GetBufferSize();

            D3D12_SHADER_BYTECODE pixelShaderBytecode = {};
            pixelShaderBytecode.pShaderBytecode = pixelShaderBlob.Get()->GetBufferPointer();
            pixelShaderBytecode.BytecodeLength = pixelShaderBlob.Get()->GetBufferSize();

            // Load compiled shaders
            psoDesc.VS = vertexShaderBytecode;
            psoDesc.PS = pixelShaderBytecode;

            // Rasterizer state (Default)
            D3D12_RASTERIZER_DESC rasterizerDesc = {};
            rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
            rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
            rasterizerDesc.FrontCounterClockwise = FALSE;
            rasterizerDesc.DepthClipEnable = TRUE;

            psoDesc.RasterizerState = rasterizerDesc;

            psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
            D3D12_DEPTH_STENCIL_DESC depthStencilDesc = {};
            depthStencilDesc.DepthEnable = TRUE;
            depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
            depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
            depthStencilDesc.StencilEnable = FALSE;

            // Attach depth-stencil state
            psoDesc.DepthStencilState = depthStencilDesc;
            psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
            psoDesc.SampleMask = UINT_MAX;
            psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
            psoDesc.NumRenderTargets = 1;
            psoDesc.RTVFormats[0] = DXGI_FORMAT_B8G8R8A8_UNORM;
            psoDesc.SampleDesc.Count = 1;
            psoDesc.SampleDesc.Quality = 0;

            D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 28, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 40, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            };

            psoDesc.InputLayout = { inputLayout, _countof(inputLayout) };


            // Create the PSO
            HRESULT hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&_pipelineState));
            if (FAILED(hr)) {
                throw std::runtime_error("Failed to create PSO");
            }
        }

        ~D3D12PipelineState() = default;

        auto GetPipelineState() const -> Microsoft::WRL::ComPtr<ID3D12PipelineState> { return _pipelineState; }

    private:
        Microsoft::WRL::ComPtr<ID3D12PipelineState> _pipelineState;
    };
}
