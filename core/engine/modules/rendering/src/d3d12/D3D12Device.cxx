#include <directx/d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include <d3d12shader.h>
#include <dxcapi.h>
#include <stdexcept>
#include <vector>
#define WIN32_LEAN_AND_MEAN
#define NO_MINMAX
#include <Windows.h>
#include <wrl.h>
#include <dxcapi.h>
#include <string>
#include <map>
#include <iostream>
#include "rendering/Constants.hxx"
#include "rendering/d3d12/D3D12Material.hxx"
#include "rendering/d3d12/D3D12Device.hxx"
#include "rendering/d3d12/D3D12GraphicsContext.hxx"
#include "rendering/d3d12/D3D12UploadContext.hxx"
#include "rendering/d3d12/D3D12CommandQueue.hxx"
#include "rendering/d3d12/D3D12Utils.hxx"
#include "rendering/d3d12/D3D12CommandAllocator.hxx"
#include "rendering/d3d12/D3D12CommandList.hxx"
#include "rendering/d3d12/D3D12SwapChain.hxx"
#include "rendering/d3d12/D3D12DepthBuffer.hxx"
#include "rendering/d3d12/D3D12RenderTarget.hxx"
#include "rendering/d3d12/D3D12ShadowMap.hxx"
#include "rendering/d3d12/D3D12IndexBuffer.hxx"
#include "rendering/d3d12/D3D12VertexBuffer.hxx"
#include "rendering/d3d12/D3D12ConstantBuffer.hxx"
#include "rendering/d3d12/D3D12StructuredBuffer.hxx"
#include "rendering/d3d12/D3D12InstanceBuffer.hxx"
#include "rendering/d3d12/D3D12Texture.hxx"
#include "rendering/d3d12/D3D12Sampler.hxx"
#include "rendering/d3d12/D3D12Cubemap.hxx"

using namespace Microsoft::WRL;

namespace playground::rendering::d3d12 {

    ComPtr<IDXGIAdapter1> GetHardwareAdapter() {
        UINT dxgiFactoryFlags = 0;
#ifdef _DEBUG
        dxgiFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif
        // Initialize DXGI factory
        ComPtr<IDXGIFactory6> dxgiFactory = nullptr;
        if (FAILED(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&dxgiFactory)))) {
            throw std::runtime_error("Failed to create DXGI factory");
        }

        for (UINT i = 0; ; ++i) {
            ComPtr<IDXGIAdapter1> adapter = nullptr;
            if (DXGI_ERROR_NOT_FOUND == dxgiFactory->EnumAdapterByGpuPreference(
                i,
                DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
                __uuidof(IDXGIAdapter1),
                &adapter)
                )

                break;

            return adapter;
        }

        throw std::runtime_error("Failed to enumerate GPU preferences");
    }

    D3D12Device::D3D12Device(uint8_t frameCount) : Device(frameCount) {
        // Create adapter
        _adapter = GetHardwareAdapter();
        // Create device
        if (_adapter == nullptr) {
            throw std::runtime_error("Failed to get hardware adapter");
        }

        DXGI_ADAPTER_DESC adapterDesc;
        // Get the adapter description (including the name)
        if (SUCCEEDED(_adapter->GetDesc(&adapterDesc))) {
            std::wcout << L"Graphics Adapter Name: " << adapterDesc.Description << std::endl;
        }

#ifdef _DEBUG
        // Enable the Debug Layer (only in Debug builds)
        Microsoft::WRL::ComPtr<ID3D12Debug> pDebugController;
        if (FAILED(D3D12GetDebugInterface(IID_PPV_ARGS(&pDebugController)))) {
            throw std::runtime_error("Failed to get debug interface");
        }
        pDebugController->EnableDebugLayer();
#endif

        if (FAILED(D3D12CreateDevice(_adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&_device))))
        {
            throw std::runtime_error("Failed to create D3D12 device");
        }

        _device->SetStablePowerState(TRUE);


        // Create heaps
        // Start with one heap per type
        // 32 RTVS (2-3 for the back buffers and 30~ for render textures)
        _rtvHeaps = std::make_unique<D3D12HeapManager>(_device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, (uint16_t)RTVHeapResource::End);
        // Use chunks of 512 entries per shader heap
        _srvHeaps = std::make_unique<D3D12HeapManager>(_device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, (uint16_t)SRVHeapResource::End);
        // 128 Depth stencils are enough for any type of render pipeline
        _dsvHeaps = std::make_unique<D3D12HeapManager>(_device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, (uint16_t)DSVHeapResource::End);

        // Create sampler heap
        // 6 Samplers:
        // - Point Clamp
        // - Point Wrap
        // - Linear Clamp
        // - Linear Wrap
        // - Anisotropic Clamp
        // - Anisotropic Wrap
        _samplerHeaps = std::make_unique<D3D12HeapManager>(_device, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, (uint16_t)SamplerHeapResource::End);

        _skyboxRootSignature = CreateSkyboxRootSignature();
        _rootSignature = CreateRootSignature();
        _shadowRootSignature = CreateShadowRootSignature();
        _postProcessingRootSignature = CreatePostprocessSignature();

        _graphicsQueue = CreateCommandQueue(CommandListType::Graphics, "GraphicsQueue");
        _computeQueue = CreateCommandQueue(CommandListType::Compute, "ComputeQueue");
        _copyQueue = CreateCommandQueue(CommandListType::Copy, "CopyQueue");
        _uploadQueue = CreateCommandQueue(CommandListType::Transfer, "UploadQueue");

#if ENABLE_PROFILER
        _tracyCtx = tracy::CreateD3D12Context(_device.Get(), _graphicsQueue.Get());
#endif
    }

    D3D12Device::~D3D12Device() {
        _adapter = nullptr;
        Flush();

#if ENABLE_PROFILER
        tracy::DestroyD3D12Context(_tracyCtx);
#endif

        _device = nullptr;

        Microsoft::WRL::ComPtr<IDXGIDebug1> dxgiDebug;
        if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug)))) {
            dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
        }
    }

    auto D3D12Device::Flush() -> void {
        _rtvHeaps = nullptr;
        _srvHeaps = nullptr;
        _dsvHeaps = nullptr;
        _samplerHeaps = nullptr;
        _rootSignature = nullptr;
        _shadowRootSignature = nullptr;
        _graphicsQueue = nullptr;
        _computeQueue = nullptr;
        _copyQueue = nullptr;
        _uploadQueue = nullptr;
        _adapter = nullptr;
    }

    auto D3D12Device::CreateGraphicsContext(std::string name, void* window, uint32_t width, uint32_t height, bool offscreen) -> std::shared_ptr<GraphicsContext>
    {
        return std::make_shared<D3D12GraphicsContext>(
            name,
            shared_from_this(),
            _skyboxRootSignature,
            _rootSignature,
            _shadowRootSignature,
            _postProcessingRootSignature,
            _graphicsQueue,
            _uploadQueue,
            window,
            width,
            height,
            offscreen
#if ENABLE_PROFILER
            , _tracyCtx
#endif
        );
    }

    auto D3D12Device::CreateUploadContext(std::string name) -> std::shared_ptr<UploadContext>
    {
        return std::make_shared<D3D12UploadContext>(
            name,
            shared_from_this(),
            _uploadQueue
        );
    }

    auto D3D12Device::CreateCommandList(
        CommandListType type,
        std::string name
    ) -> std::shared_ptr<CommandList>
    {
        return std::make_shared<D3D12CommandList>(shared_from_this(), type, name);
    }

    auto D3D12Device::CreateCommandQueue(
        CommandListType type,
        std::string name
    ) -> ComPtr<ID3D12CommandQueue>
    {
        ComPtr<ID3D12CommandQueue> queue = nullptr;
        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        switch (type)
        {
        case CommandListType::Graphics:
            queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
            break;
        case CommandListType::Compute:
            queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
            break;
        case CommandListType::Copy:
            queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
            break;
        case CommandListType::Transfer:
            queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
            break;
        }

        queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

        if (FAILED(_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&queue)))) {
            throw std::runtime_error("Failed to create command queue");
        }

        queue->SetName(std::wstring(name.begin(), name.end()).c_str());

        return queue;
    }

    auto D3D12Device::CreateBuffer(uint64_t size) -> std::shared_ptr<Buffer>
    {
        return nullptr;
    }

    auto D3D12Device::CreateRenderTarget(
        uint32_t width,
        uint32_t height,
        TextureFormat format,
        std::string name,
        bool isCPUReadable
    ) -> std::shared_ptr<RenderTarget>
    {
        ComPtr<ID3D12Resource> rtv = nullptr;
        // Describe the texture resource for the render target.
        D3D12_RESOURCE_DESC rtDesc = {};
        rtDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        rtDesc.Alignment = 0;
        rtDesc.Width = width;
        rtDesc.Height = height;
        rtDesc.DepthOrArraySize = 1;
        rtDesc.MipLevels = 1;
        rtDesc.Format = DXGIFormatFrom(format);
        rtDesc.SampleDesc.Count = 1;
        rtDesc.SampleDesc.Quality = 0;
        rtDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        rtDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

        if (isCPUReadable) {
            rtDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        }

        D3D12_CLEAR_VALUE clearValue = {};
        clearValue.Format = DXGIFormatFrom(format);
        clearValue.Color[0] = 0.2f;
        clearValue.Color[1] = 0.0f;
        clearValue.Color[2] = 0.7f;
        clearValue.Color[3] = 1.0f;

        auto handle = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        if (FAILED(_device->CreateCommittedResource(
            &handle,
            D3D12_HEAP_FLAG_NONE,
            &rtDesc,
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            &clearValue,
            IID_PPV_ARGS(&rtv)
        ))) {
            throw std::runtime_error("Failed to create render target");
        }

        auto nextHandle = _rtvHeaps->NextHandle(RTVHeapResource::RenderTargetView);

        _device->CreateRenderTargetView(rtv.Get(), nullptr, nextHandle->GetCPUHandle());

        rtv->SetName(std::wstring(name.begin(), name.end()).c_str());

        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format = DXGIFormatFrom(format);
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels = 1;
        srvDesc.Texture2D.PlaneSlice = 0;
        srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

        auto handleSrv = _srvHeaps->NextHandle(SRVHeapResource::FrameBuffer);
        _device->CreateShaderResourceView(rtv.Get(), &srvDesc, handleSrv->GetCPUHandle());

        return std::make_shared<D3D12RenderTarget>(rtv, nextHandle->GetCPUHandle(), handleSrv);
    }

    auto D3D12Device::CreateDepthBuffer(
        uint32_t width,
        uint32_t height,
        std::string name
    ) -> std::shared_ptr<DepthBuffer>
    {
        ComPtr<ID3D12Resource> depthBuffer = nullptr;
        // Describe the texture resource for the render target.
        D3D12_RESOURCE_DESC rtDesc = {};
        rtDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        rtDesc.Alignment = 0;
        rtDesc.Width = width;
        rtDesc.Height = height;
        rtDesc.DepthOrArraySize = 1;
        rtDesc.MipLevels = 1;
        rtDesc.Format = DXGI_FORMAT_R32_TYPELESS;
        rtDesc.SampleDesc.Count = 1;
        rtDesc.SampleDesc.Quality = 0;
        rtDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

        D3D12_CLEAR_VALUE clearValue = {};
        clearValue.Format = DXGI_FORMAT_D32_FLOAT;
        clearValue.DepthStencil.Depth = 1.0f;
        clearValue.DepthStencil.Stencil = 0;

        auto handle = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        if (FAILED(_device->CreateCommittedResource(
            &handle,
            D3D12_HEAP_FLAG_NONE,
            &rtDesc,
            D3D12_RESOURCE_STATE_DEPTH_WRITE,
            &clearValue,
            IID_PPV_ARGS(&depthBuffer)
        ))) {
            throw std::runtime_error("Failed to create depth buffer");
        }

        auto nextHandle = _dsvHeaps->NextHandle(DSVHeapResource::DepthStencilView);
        depthBuffer->SetName(std::wstring(name.begin(), name.end()).c_str());

        D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
        dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
        dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

        D3D12_DEPTH_STENCIL_VIEW_DESC dsvDescReadonly = {};
        dsvDescReadonly.Format = DXGI_FORMAT_D32_FLOAT;
        dsvDescReadonly.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        dsvDescReadonly.Flags = D3D12_DSV_FLAG_READ_ONLY_DEPTH;

        _device->CreateDepthStencilView(depthBuffer.Get(), &dsvDesc, nextHandle->GetCPUHandle());

        auto readOnlyHandle = _dsvHeaps->NextHandle(DSVHeapResource::DepthStencilView);
        _device->CreateDepthStencilView(depthBuffer.Get(), &dsvDescReadonly, readOnlyHandle->GetCPUHandle());

        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels = 1;
        srvDesc.Texture2D.PlaneSlice = 0;
        srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

        auto handleSrv = _srvHeaps->NextHandle(SRVHeapResource::DepthBuffer);

        _device->CreateShaderResourceView(depthBuffer.Get(), &srvDesc, handleSrv->GetCPUHandle());

        return std::make_shared<D3D12DepthBuffer>(depthBuffer, nextHandle->GetCPUHandle(), readOnlyHandle->GetCPUHandle(), handleSrv);
    }

    auto D3D12Device::CreateMaterial(std::string vertexShader, std::string pixelShader, MaterialType type) -> std::shared_ptr<Material>
    {
        Microsoft::WRL::ComPtr<ID3D12RootSignature> sig = nullptr;
        Microsoft::WRL::ComPtr<ID3D12PipelineState> pso = nullptr;

        if (type == MaterialType::Shadow) {
            sig = _shadowRootSignature;
            pso = CreateShadowPipelineState(vertexShader, sig);
        } else if (type == MaterialType::Skybox) {
            sig = _skyboxRootSignature;
            pso = CreateSkyboxPipelineState(vertexShader, pixelShader, sig);
        } else if (type == MaterialType::Standard) {
            sig = _rootSignature;
            pso = CreatePipelineState(vertexShader, pixelShader, sig);
        } else if (type == MaterialType::PostProcessing) {
            sig = _postProcessingRootSignature;
            pso = CreatePostprocessingPipelineState(vertexShader, pixelShader, sig);
        } else {
            throw std::runtime_error("Unsupported material type");
        }

        auto material = std::make_shared<D3D12Material>(pso);

        return material;
    }

    auto D3D12Device::CreatePipelineState(const std::string& vertexShader, const std::string& pixelShader, Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature) -> Microsoft::WRL::ComPtr<ID3D12PipelineState>
    {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

        psoDesc.pRootSignature = rootSignature.Get();

        D3D12_SHADER_BYTECODE vertexShaderBytecode = {};
        vertexShaderBytecode.pShaderBytecode = vertexShader.data();
        vertexShaderBytecode.BytecodeLength = vertexShader.size();
        psoDesc.VS = vertexShaderBytecode;

        D3D12_SHADER_BYTECODE pixelShaderBytecode = {};
        pixelShaderBytecode.pShaderBytecode = pixelShader.data();
        pixelShaderBytecode.BytecodeLength = pixelShader.size();

        // Load compiled shaders
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
            { "INSTANCE_TRANSFORM", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
            { "INSTANCE_TRANSFORM", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
            { "INSTANCE_TRANSFORM", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
            { "INSTANCE_TRANSFORM", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
            { "INSTANCE_NORMALS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 64, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
            { "INSTANCE_NORMALS", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 80, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
            { "INSTANCE_NORMALS", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 96, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
            { "INSTANCE_NORMALS", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 112, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 }
        };

        psoDesc.InputLayout = { inputLayout, _countof(inputLayout) };

        Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState;
        // Create the PSO
        HRESULT hr = _device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState));
        if (FAILED(hr)) {
            throw std::runtime_error("Failed to create PSO");
        }

        return pipelineState;
    }

    auto D3D12Device::CreateShadowPipelineState(const std::string& vertexShader, Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature)
        -> Microsoft::WRL::ComPtr<ID3D12PipelineState> {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

        psoDesc.pRootSignature = rootSignature.Get();

        D3D12_SHADER_BYTECODE vertexShaderBytecode = {};
        vertexShaderBytecode.pShaderBytecode = vertexShader.data();
        vertexShaderBytecode.BytecodeLength = vertexShader.size();
        psoDesc.VS = vertexShaderBytecode;

        // Rasterizer state (Default)
        D3D12_RASTERIZER_DESC rasterizerDesc = {};
        rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
        rasterizerDesc.CullMode = D3D12_CULL_MODE_FRONT;
        rasterizerDesc.FrontCounterClockwise = FALSE;
        rasterizerDesc.DepthClipEnable = TRUE;

        rasterizerDesc.DepthBias = 0;
        rasterizerDesc.DepthBiasClamp = 0;
        rasterizerDesc.SlopeScaledDepthBias = -0.25f;
        

        psoDesc.RasterizerState = rasterizerDesc;

        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        D3D12_DEPTH_STENCIL_DESC depthStencilDesc = {};
        depthStencilDesc.DepthEnable = TRUE;
        depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
        depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
        depthStencilDesc.StencilEnable = FALSE;

        // Attach depth-stencil state
        psoDesc.DepthStencilState = depthStencilDesc;
        psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 0;
        psoDesc.SampleDesc.Count = 1;
        psoDesc.SampleDesc.Quality = 0;

        D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 28, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 40, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "INSTANCE_TRANSFORM", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
            { "INSTANCE_TRANSFORM", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
            { "INSTANCE_TRANSFORM", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
            { "INSTANCE_TRANSFORM", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
            { "INSTANCE_NORMALS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 64, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
            { "INSTANCE_NORMALS", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 80, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
            { "INSTANCE_NORMALS", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 96, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
            { "INSTANCE_NORMALS", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 112, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 }
        };

        psoDesc.InputLayout = { inputLayout, _countof(inputLayout) };

        Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState;
        // Create the PSO
        HRESULT hr = _device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState));
        if (FAILED(hr)) {
            throw std::runtime_error("Failed to create PSO");
        }

        return pipelineState;
    }

    auto D3D12Device::CreateSkyboxPipelineState(const std::string& vertexShader, const std::string& pixelShader, Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature)
        -> Microsoft::WRL::ComPtr<ID3D12PipelineState> {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

        psoDesc.pRootSignature = rootSignature.Get();

        D3D12_SHADER_BYTECODE vertexShaderBytecode = {};
        vertexShaderBytecode.pShaderBytecode = vertexShader.data();
        vertexShaderBytecode.BytecodeLength = vertexShader.size();
        psoDesc.VS = vertexShaderBytecode;

        D3D12_SHADER_BYTECODE pixelShaderBytecode = {};
        pixelShaderBytecode.pShaderBytecode = pixelShader.data();
        pixelShaderBytecode.BytecodeLength = pixelShader.size();

        // Load compiled shaders
        psoDesc.PS = pixelShaderBytecode;

        // Rasterizer state (Default)
        D3D12_RASTERIZER_DESC rasterizerDesc = {};
        rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
        rasterizerDesc.CullMode = D3D12_CULL_MODE_FRONT;
        rasterizerDesc.FrontCounterClockwise = FALSE;
        rasterizerDesc.DepthClipEnable = FALSE;
        psoDesc.RasterizerState = rasterizerDesc;

        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        D3D12_DEPTH_STENCIL_DESC depthStencilDesc = {};
        depthStencilDesc.DepthEnable = FALSE;
        depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;;
        depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
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

        Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState;
        // Create the PSO
        HRESULT hr = _device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState));
        if (FAILED(hr)) {
            throw std::runtime_error("Failed to create PSO");
        }

        return pipelineState;
    }

    auto D3D12Device::CreatePostprocessingPipelineState(
        const std::string& vertexShader,
        const std::string& pixelShader,
        Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature
    ) -> Microsoft::WRL::ComPtr<ID3D12PipelineState> {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

        psoDesc.pRootSignature = rootSignature.Get();

        D3D12_SHADER_BYTECODE vertexShaderBytecode = {};
        vertexShaderBytecode.pShaderBytecode = vertexShader.data();
        vertexShaderBytecode.BytecodeLength = vertexShader.size();
        psoDesc.VS = vertexShaderBytecode;

        D3D12_SHADER_BYTECODE pixelShaderBytecode = {};
        pixelShaderBytecode.pShaderBytecode = pixelShader.data();
        pixelShaderBytecode.BytecodeLength = pixelShader.size();

        // Load compiled shaders
        psoDesc.PS = pixelShaderBytecode;

        // Rasterizer state (Default)
        D3D12_RASTERIZER_DESC rasterizerDesc = {};
        rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
        rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
        rasterizerDesc.FrontCounterClockwise = TRUE;
        rasterizerDesc.DepthClipEnable = FALSE;
        psoDesc.RasterizerState = rasterizerDesc;

        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        D3D12_DEPTH_STENCIL_DESC depthStencilDesc = {};
        depthStencilDesc.DepthEnable = FALSE;
        depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;;
        depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
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

        Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState;
        // Create the PSO
        HRESULT hr = _device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState));
        if (FAILED(hr)) {
            throw std::runtime_error("Failed to create PSO");
        }

        return pipelineState;
    }

    auto D3D12Device::CreateVertexBuffer(const void* data, uint64_t size, uint64_t stride, bool isStatic) -> std::shared_ptr<VertexBuffer>
    {
        auto buffer = std::make_shared<D3D12VertexBuffer>(_device, data, size, stride, isStatic);

        return buffer;
    }

    auto D3D12Device::UpdateVertexBuffer(std::shared_ptr<VertexBuffer> buffer, const void* data, uint64_t size) -> void
    {
    }

    auto D3D12Device::CreateIndexBuffer(const uint32_t* indices, size_t size) -> std::shared_ptr<IndexBuffer>
    {
        auto buffer = std::make_shared<D3D12IndexBuffer>(_device, indices, size);

        return buffer;
    }

    auto D3D12Device::UpdateIndexBuffer(std::shared_ptr<IndexBuffer> buffer, std::vector<uint32_t> indices) -> void
    {

    }

    auto D3D12Device::CreateTexture(uint32_t width, uint32_t height, std::vector<std::vector<uint8_t>> mips, Allocator& allocator) -> std::shared_ptr<Texture> {
        return std::make_shared<D3D12Texture>(_device, width, height, mips, _srvHeaps->NextHandle(SRVHeapResource::Texture2D), allocator);
    }

    auto D3D12Device::CreateCubemap(uint32_t width, uint32_t height, std::vector<std::vector<std::vector<uint8_t>>> faces, Allocator& allocator) -> std::shared_ptr<Cubemap> {
        return std::make_shared<D3D12Cubemap>(_device, width, height, faces, _srvHeaps->NextHandle(SRVHeapResource::TextureCube), allocator);
    }

    auto D3D12Device::CreateSampler(TextureFiltering filtering, TextureWrapping wrapping) -> std::shared_ptr<Sampler> {
        return std::make_shared<D3D12TextureSampler>(_device, _samplerHeaps->NextHandle(SamplerHeapResource::Sampler), filtering, wrapping);
    }

    auto D3D12Device::CreateShadowMap(uint32_t width, uint32_t height, std::string name)->std::shared_ptr<ShadowMap> {
        auto dsvHandle = _dsvHeaps->NextHandle(DSVHeapResource::ShadowMapDSV);
        auto srvHandle = _srvHeaps->NextHandle(SRVHeapResource::ShadowMap);

        return std::make_shared<D3D12ShadowMap>(GetDevice(), dsvHandle, srvHandle, width, height, name);
    }

    auto D3D12Device::CreateSwapchain(uint8_t bufferCount, uint16_t width, uint16_t height, void* window) -> std::shared_ptr<Swapchain> {
        return std::make_shared<D3D12SwapChain>(bufferCount, _graphicsQueue, width, height, (HWND)window);
    }

    auto D3D12Device::GetSrvHeap() -> std::shared_ptr<Heap> {
        return _srvHeaps->Heap();
    }

    auto D3D12Device::GetSamplerHeap() -> std::shared_ptr<Heap> {
        return _samplerHeaps->Heap();
    }

    auto D3D12Device::CreateConstantBuffer(const void* data, size_t size, size_t itemSize, ConstantBuffer::BindingMode mode, std::string name) -> std::shared_ptr<ConstantBuffer> {
        auto handle = _srvHeaps->NextHandle(SRVHeapResource::ConstantBuffer);
        return std::make_shared<D3D12ConstantBuffer>(
            _device,
            handle->GetCPUHandle(),
            handle->GetGPUHandle(),
            _srvHeaps->Heap()->Native(),
            data,
            size,
            (itemSize + 255) & ~255,
            mode,
            name
        );
    }

    auto D3D12Device::CreateStructuredBuffer(void* data, size_t size, size_t itemSize, std::string name) -> std::shared_ptr<StructuredBuffer> {
        auto handle = _srvHeaps->NextHandle(SRVHeapResource::StructuredBuffer);
        return std::make_shared<D3D12StructuredBuffer>(
            _device,
            handle->GetCPUHandle(),
            handle->GetGPUHandle(),
            _srvHeaps->Heap()->Native(),
            data,
            size,
            itemSize,
            name
        );
    }

    auto D3D12Device::CreateInstanceBuffer(uint64_t count, uint64_t stride) -> std::shared_ptr<InstanceBuffer> {
        auto buffer = std::make_shared<D3D12InstanceBuffer>(_device, count, stride);

        return buffer;
    }

    auto D3D12Device::CreateSkyboxRootSignature() -> Microsoft::WRL::ComPtr<ID3D12RootSignature> {
        std::array<CD3DX12_ROOT_PARAMETER, 4> rootParameters = {};

        uint16_t cubemapsCount = (uint16_t)SRVHeapResource::ShadowMap;
        cubemapsCount -= (uint16_t)SRVHeapResource::TextureCube;
        CD3DX12_DESCRIPTOR_RANGE cubemapRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, cubemapsCount, 0, 0, 0);
        rootParameters[0].InitAsDescriptorTable(1, &cubemapRange, D3D12_SHADER_VISIBILITY_PIXEL);
        rootParameters[1].InitAsConstants(
            2,
            0,
            0 
        );

        rootParameters[2].InitAsConstantBufferView(
            1,
            0
        );

        rootParameters[MATERIAL_BUFFER_BINDING].InitAsConstantBufferView(
            2,
            0
        );

        const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
            0,
            D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT,
            D3D12_TEXTURE_ADDRESS_MODE_BORDER,
            D3D12_TEXTURE_ADDRESS_MODE_BORDER,
            D3D12_TEXTURE_ADDRESS_MODE_BORDER,
            0.0f,
            16,
            D3D12_COMPARISON_FUNC_ALWAYS,
            D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE
        );


        // Define the root signature descriptor
        CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(
            rootParameters.size(),
            rootParameters.data(),
            1, &pointClamp,
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
        );

        // Serialize and create the root signature
        Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob;
        Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
        HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);

        if (FAILED(hr)) {
            if (errorBlob) {
                OutputDebugStringA((char*)errorBlob->GetBufferPointer());
            }
            throw std::runtime_error("Failed to serialize root signature");
        }

        Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
        hr = _device->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
        if (FAILED(hr)) {
            throw std::runtime_error("Failed to create root signature");
        }

        return rootSignature;
    }

    auto D3D12Device::CreateRootSignature() -> Microsoft::WRL::ComPtr<ID3D12RootSignature> {
        std::array<CD3DX12_ROOT_PARAMETER, 9> rootParameters = {};

        // Per-frame CBV (Camera, Lighting, Material props, etc.)
        rootParameters[GLOBALS_BUFFER_BINDING].InitAsConstantBufferView(GLOBALS_BUFFER_BINDING); // b0
        rootParameters[DIRECTIONAL_LIGHT_BUFFER_BINDING].InitAsConstantBufferView(DIRECTIONAL_LIGHT_BUFFER_BINDING); // b1
        rootParameters[CAMERA_BUFFER_BINDING].InitAsConstantBufferView(CAMERA_BUFFER_BINDING); // b2
        rootParameters[MATERIAL_BUFFER_BINDING].InitAsConstantBufferView(MATERIAL_BUFFER_BINDING); // b3
        rootParameters[SHADOW_CASTERS_COUNT_BINDING].InitAsConstants(1, SHADOW_CASTERS_COUNT_BINDING, 0); // b4

        uint16_t texturesCount = (uint16_t)SRVHeapResource::TextureCube;
        texturesCount -= (uint16_t)SRVHeapResource::Texture2D;
        CD3DX12_DESCRIPTOR_RANGE texRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, texturesCount, 0, 0, 0);
        rootParameters[BINDLESS_TEXTURES_SLOT].InitAsDescriptorTable(1, &texRange, D3D12_SHADER_VISIBILITY_PIXEL);

        uint16_t cubemapsCount = (uint16_t)SRVHeapResource::ShadowMap;
        cubemapsCount -= (uint16_t)SRVHeapResource::TextureCube;
        CD3DX12_DESCRIPTOR_RANGE cubemapRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, cubemapsCount, 0, 1, 0);
        rootParameters[BINDLESS_CUBEMAPS_SLOT].InitAsDescriptorTable(1, &cubemapRange, D3D12_SHADER_VISIBILITY_PIXEL);

        uint16_t shadowMapsCount = (uint16_t)SRVHeapResource::StructuredBuffer;
        shadowMapsCount -= (uint16_t)SRVHeapResource::ShadowMap;
        CD3DX12_DESCRIPTOR_RANGE shadowMapsRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, shadowMapsCount, 0, 2, 0);
        rootParameters[BINDLESS_SHADOW_MAPS_SLOT].InitAsDescriptorTable(1, &shadowMapsRange, D3D12_SHADER_VISIBILITY_PIXEL);

        CD3DX12_DESCRIPTOR_RANGE shadowMapsBuffer(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 3, 0);
        rootParameters[SHADOW_CASTERS_BUFFER_BINDING].InitAsDescriptorTable(1, &shadowMapsBuffer, D3D12_SHADER_VISIBILITY_PIXEL);

        const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
            0,
            D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT,
            D3D12_TEXTURE_ADDRESS_MODE_BORDER,
            D3D12_TEXTURE_ADDRESS_MODE_BORDER,
            D3D12_TEXTURE_ADDRESS_MODE_BORDER,
            0.0f,
            16,
            D3D12_COMPARISON_FUNC_ALWAYS,
            D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE
        );

        const CD3DX12_STATIC_SAMPLER_DESC shadow(
            1,
            D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT,
            D3D12_TEXTURE_ADDRESS_MODE_BORDER,
            D3D12_TEXTURE_ADDRESS_MODE_BORDER,
            D3D12_TEXTURE_ADDRESS_MODE_BORDER,
            0.0f,
            16,
            D3D12_COMPARISON_FUNC_LESS_EQUAL,
            D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE
        );

        std::array<CD3DX12_STATIC_SAMPLER_DESC, 2> staticSamplers = { pointClamp, shadow };

        // Define the root signature descriptor
        CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(
            rootParameters.size(),
            rootParameters.data(),
            staticSamplers.size(), staticSamplers.data(),
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
        );

        // Serialize and create the root signature
        Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob;
        Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
        HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);

        if (FAILED(hr)) {
            if (errorBlob) {
                OutputDebugStringA((char*)errorBlob->GetBufferPointer());
            }
            throw std::runtime_error("Failed to serialize root signature");
        }

        Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
        hr = _device->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
        if (FAILED(hr)) {
            throw std::runtime_error("Failed to create root signature");
        }

        return rootSignature;
    }

    auto D3D12Device::CreateShadowRootSignature() -> Microsoft::WRL::ComPtr<ID3D12RootSignature> {
        std::array<CD3DX12_ROOT_PARAMETER, 1> rootParameters = {};

        // 1️ Per-frame CBV (Camera, Lighting, Material props, etc.)
        rootParameters[0].InitAsConstantBufferView(0); // b0

        // Define the root signature descriptor
        CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(
            rootParameters.size(),
            rootParameters.data(),
            0, nullptr, // No static samplers
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
        );

        // Serialize and create the root signature
        Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob;
        Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
        HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);

        if (FAILED(hr)) {
            throw std::runtime_error("Failed to serialize root signature");
        }

        Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
        hr = _device->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
        if (FAILED(hr)) {
            throw std::runtime_error("Failed to create root signature");
        }

        return rootSignature;
    }

    auto D3D12Device::CreatePostprocessSignature() -> Microsoft::WRL::ComPtr<ID3D12RootSignature> {
        std::array<CD3DX12_ROOT_PARAMETER, 8> rootParameters = {};

        // Per-frame CBV (Camera, Lighting, Material props, etc.)
        rootParameters[GLOBALS_BUFFER_BINDING].InitAsConstantBufferView(GLOBALS_BUFFER_BINDING); // b0
        rootParameters[PP_INVERSE_SCREEN_RES_CONSTANTS_BINDING].InitAsConstants(2, PP_INVERSE_SCREEN_RES_CONSTANTS_BINDING, 0); // b1
        rootParameters[CAMERA_BUFFER_BINDING].InitAsConstantBufferView(CAMERA_BUFFER_BINDING); // b2
        rootParameters[MATERIAL_BUFFER_BINDING].InitAsConstantBufferView(MATERIAL_BUFFER_BINDING); // b3

        CD3DX12_DESCRIPTOR_RANGE colourRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);
        rootParameters[PP_FRAME_COLOUR_BUFFER_BINDING].InitAsDescriptorTable(1, &colourRange, D3D12_SHADER_VISIBILITY_PIXEL); // t0
        CD3DX12_DESCRIPTOR_RANGE depthRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 1);
        rootParameters[PP_FRAME_DEPTH_BUFFER_BINDING].InitAsDescriptorTable(1, &depthRange, D3D12_SHADER_VISIBILITY_PIXEL); // t1

        uint16_t texturesCount = (uint16_t)SRVHeapResource::TextureCube;
        texturesCount -= (uint16_t)SRVHeapResource::Texture2D;
        CD3DX12_DESCRIPTOR_RANGE texRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, texturesCount, 0, 2);
        rootParameters[PP_BINDLESS_TEXTURES_SLOT].InitAsDescriptorTable(1, &texRange, D3D12_SHADER_VISIBILITY_PIXEL);

        uint16_t cubemapsCount = (uint16_t)SRVHeapResource::ShadowMap;
        cubemapsCount -= (uint16_t)SRVHeapResource::TextureCube;
        CD3DX12_DESCRIPTOR_RANGE cubemapRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, cubemapsCount, 0, 3);
        rootParameters[PP_BINDLESS_CUBEMAPS_SLOT].InitAsDescriptorTable(1, &cubemapRange, D3D12_SHADER_VISIBILITY_PIXEL);

        const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
            0,
            D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT,
            D3D12_TEXTURE_ADDRESS_MODE_BORDER,
            D3D12_TEXTURE_ADDRESS_MODE_BORDER,
            D3D12_TEXTURE_ADDRESS_MODE_BORDER,
            0.0f,
            16,
            D3D12_COMPARISON_FUNC_ALWAYS,
            D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE
        );

        // Define the root signature descriptor
        CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(
            rootParameters.size(),
            rootParameters.data(),
            1, &pointClamp,
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
        );

        // Serialize and create the root signature
        Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob;
        Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
        HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);

        if (FAILED(hr)) {
            if (errorBlob) {
                OutputDebugStringA((char*)errorBlob->GetBufferPointer());
            }
            throw std::runtime_error("Failed to serialize root signature");
        }

        Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
        hr = _device->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
        if (FAILED(hr)) {
            throw std::runtime_error("Failed to create root signature");
        }

        return rootSignature;
    }

    auto D3D12Device::DestroyShader(uint64_t shaderHandle) -> void
    {
    }

    auto D3D12Device::WaitForIdleGPU() -> void {
        Microsoft::WRL::ComPtr<ID3D12Fence> fence;
        _device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));

        HANDLE fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        UINT64 fenceValue = 1;

        // Signal the GPU to raise the fence when it’s done
        _graphicsQueue->Signal(fence.Get(), fenceValue);

        // Wait until the GPU hits the fence
        if (fence->GetCompletedValue() < fenceValue) {
            fence->SetEventOnCompletion(fenceValue, fenceEvent);
            WaitForSingleObject(fenceEvent, INFINITE);
        }

        CloseHandle(fenceEvent);
    }
}
