#include <directx/d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include <stdexcept>
#include <vector>
#include <Windows.h>
#include <wrl.h>
#include "rendering/d3d12/D3D12Device.hxx"
#include "rendering/d3d12/D3D12GraphicsContext.hxx"
#include "rendering/d3d12/D3D12CommandQueue.hxx"
#include "rendering/d3d12/D3D12Utils.hxx"
#include "rendering/d3d12/D3D12CommandAllocator.hxx"
#include "rendering/d3d12/D3D12CommandList.hxx"
#include "rendering/d3d12/D3D12SwapChain.hxx"
#include "rendering/d3d12/D3D12DepthBuffer.hxx"
#include "rendering/d3d12/D3D12RenderTarget.hxx"


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
                0,
                DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
                __uuidof(IDXGIAdapter1),
                &adapter)
                )

                break;

            return adapter;
        }

        throw std::runtime_error("Failed to enumerate GPU preferences");
    }

    D3D12Device::D3D12Device(void* window, uint8_t frameCount) : Device(frameCount) {
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

        // Create heaps
        // Start with one heap per type
        // 32 RTVS (2-3 for the back buffers and 30~ for render textures)
        _rtvHeaps = std::make_unique<D3D12HeapManager>(_device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 32);
        // Use chunks of 512 entries per shader heap
        _srvHeaps = std::make_unique<D3D12HeapManager>(_device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 512);
        // 32 Depth stencils are enough for any type of render pipeline
        _dsvHeaps = std::make_unique<D3D12HeapManager>(_device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 32);
    }

    D3D12Device::~D3D12Device() {
        _adapter = nullptr;
        Flush();
        Microsoft::WRL::ComPtr<IDXGIDebug1> dxgiDebug;
        if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug)))) {
            dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
        }
    }

    auto D3D12Device::Flush() -> void {
        _rtvHeaps = nullptr;
        _srvHeaps = nullptr;
        _dsvHeaps = nullptr;
    }

    auto D3D12Device::CreateGraphicsContext(void* window, uint32_t width, uint32_t height) -> std::unique_ptr<Context>
    {
        return std::make_unique<D3D12GraphicsContext>(
            _device,
            CreateCommandQueue(CommandListType::Graphics, "GraphicsQueue"),
            window,
            width,
            height
        );
    }

    auto D3D12Device::CreateUploadContext() -> std::unique_ptr<Context>
    {
        return nullptr;
    }

    auto D3D12Device::CreateCommandList(
        CommandListType type,
        std::string name
    ) -> std::shared_ptr<CommandList>
    {
        return std::make_shared<D3D12CommandList>(_device.Get(), type, _frameCount, name);
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

    auto D3D12Device::CreateFence() -> std::shared_ptr<Fence>
    {
        return nullptr;
    }

    auto D3D12Device::CreateBuffer(uint64_t size) -> std::shared_ptr<Buffer>
    {
        return nullptr;
    }

    auto D3D12Device::CreateRenderTarget(
        uint32_t width,
        uint32_t height,
        TextureFormat format,
        std::string name
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

        D3D12_CLEAR_VALUE clearValue = {};
        clearValue.Format = DXGIFormatFrom(format);
        clearValue.Color[0] = 0.0f;
        clearValue.Color[1] = 0.0f;
        clearValue.Color[2] = 0.0f;
        clearValue.Color[3] = 1.0f;

        auto handle = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        if(FAILED(_device->CreateCommittedResource(
            &handle,
            D3D12_HEAP_FLAG_NONE,
            &rtDesc,
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            &clearValue,
            IID_PPV_ARGS(&rtv)
        ))) {
            throw std::runtime_error("Failed to create render target");
        }

        auto nextCpuHandle = _rtvHeaps->NextCpuHandle();
        auto native = nextCpuHandle->GetHandle();

        _device->CreateRenderTargetView(rtv.Get(), nullptr, native);

        if (!name.empty()) {
            rtv->SetName(std::wstring(name.begin(), name.end()).c_str());
        }

        return std::make_shared<D3D12RenderTarget>(rtv, native);
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
        rtDesc.Format = DXGI_FORMAT_D32_FLOAT;
        rtDesc.SampleDesc.Count = 1;
        rtDesc.SampleDesc.Quality = 0;
        rtDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
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

        auto nextCpuHandle = _dsvHeaps->NextCpuHandle();
        auto native = nextCpuHandle->GetHandle();

        if (!name.empty()) {
            depthBuffer->SetName(std::wstring(name.begin(), name.end()).c_str());
        }

        _device->CreateDepthStencilView(depthBuffer.Get(), nullptr, native);

        return std::make_shared<D3D12DepthBuffer>(depthBuffer, native);
    }

    auto D3D12Device::CreateMaterial(std::map<ShaderType, std::shared_ptr<Shader>> shaders,
        std::map<std::string, uint64_t> textures, std::map<std::string, float> floats,
        std::map<std::string, uint32_t> ints, std::map<std::string, bool> bools,
        std::map<std::string, std::array<float, 2>> vec2s, std::map<std::string, std::array<float, 3>> vec3s,
        std::map<std::string, std::array<float, 4>> vec4s) -> std::shared_ptr<Material>
    {
        return nullptr;
    }

    auto D3D12Device::CompileShader(const std::string& shaderSource, ShaderType type) -> std::shared_ptr<Shader>
    {
        return nullptr;
    }

    auto D3D12Device::CreateVertexBuffer(const void* data, uint64_t size) -> std::shared_ptr<VertexBuffer>
    {
        return nullptr;
    }

    auto D3D12Device::UpdateVertexBuffer(std::shared_ptr<VertexBuffer> buffer, const void* data, uint64_t size) -> void
    {
    }

    auto D3D12Device::CreateIndexBuffer(const uint32_t* indices, size_t len) -> std::shared_ptr<IndexBuffer>
    {
        return nullptr;
    }

    auto D3D12Device::UpdateIndexBuffer(std::shared_ptr<IndexBuffer> buffer, std::vector<uint32_t> indices) -> void
    {

    }

    auto D3D12Device::DestroyShader(uint64_t shaderHandle) -> void
    {
    }
}
