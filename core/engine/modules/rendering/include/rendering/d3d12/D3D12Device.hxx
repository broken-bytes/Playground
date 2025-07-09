#pragma once

#include <cstdint>
#include <directx/d3dx12.h>
#include <dxgi.h>
#include <dxgi1_4.h>
#include <dxgi1_5.h>
#include <wrl.h>
#include <string>
#include "rendering/Device.hxx"
#include "rendering/d3d12/D3D12HeapManager.hxx"
#include <tracy/Tracy.hpp>
#include <tracy/TracyD3D12.hpp>

namespace playground::rendering::d3d12 {
    class D3D12Device : public rendering::Device, public std::enable_shared_from_this<D3D12Device> {
    public:
        explicit D3D12Device(uint8_t frameCount);
        ~D3D12Device();

        auto Flush() -> void override;
        auto CreateGraphicsContext(std::string name, void* window, uint32_t width, uint32_t height, bool offscreen) -> std::shared_ptr<GraphicsContext> override;
        auto CreateUploadContext(std::string name) -> std::shared_ptr<UploadContext> override;
        auto CreateCommandList(
            CommandListType type,
            std::string name
        ) -> std::shared_ptr<CommandList> override;
        auto CreateBuffer(uint64_t size) -> std::shared_ptr<Buffer> override;
        auto CreateRenderTarget(
            uint32_t width,
            uint32_t height,
            TextureFormat format,
            std::string name,
            bool isCPUReadable
        ) -> std::shared_ptr<RenderTarget> override;
        auto CreateDepthBuffer(
            uint32_t width,
            uint32_t height,
            std::string name
        ) -> std::shared_ptr<DepthBuffer> override;
        auto CreateMaterial(std::string& vertexShader, std::string& pixelShader) -> std::shared_ptr<Material> override;
        auto CreatePipelineState(const std::string& vertexShader, const std::string& pixelShader, Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature)
            -> Microsoft::WRL::ComPtr<ID3D12PipelineState>;
        auto CreateVertexBuffer(const void* data, uint64_t size, uint64_t stride, bool isStatic) -> std::shared_ptr<VertexBuffer> override;
        auto UpdateVertexBuffer(std::shared_ptr<VertexBuffer> buffer, const void* data, uint64_t size) -> void override;
        auto CreateIndexBuffer(const uint32_t* indices, size_t size) -> std::shared_ptr<IndexBuffer> override;
        auto UpdateIndexBuffer(std::shared_ptr<IndexBuffer> buffer, std::vector<uint32_t> indices) -> void override;
        auto CreateTexture(uint32_t width, uint32_t height, std::vector<std::vector<uint8_t>> mips, Allocator& allocator) -> std::shared_ptr<Texture> override;
        auto CreateSampler(TextureFiltering filtering, TextureWrapping wrapping) -> std::shared_ptr<Sampler> override;
        auto CreateSwapchain(uint8_t bufferCount, uint16_t width, uint16_t height, void* window)->std::shared_ptr<Swapchain> override;
        auto CreateConstantBuffer(const void* data, size_t size, size_t itemSize, ConstantBuffer::BindingMode mode, std::string name) -> std::shared_ptr<ConstantBuffer> override;
        auto CreateStructuredBuffer(void* data, size_t size, size_t itemSize, std::string name) -> std::shared_ptr<StructuredBuffer> override;
        auto CreateInstanceBuffer(uint64_t count, uint64_t stride) -> std::shared_ptr<InstanceBuffer> override;

        auto CreateRootSignature() -> Microsoft::WRL::ComPtr<ID3D12RootSignature>;
        auto DestroyShader(uint64_t shaderHandle) -> void override;
        auto WaitForIdleGPU() -> void override;

        auto GetSrvHeap()->std::shared_ptr<Heap> override;
        auto GetSamplerHeap()->std::shared_ptr<Heap> override;

        auto GetDevice() -> Microsoft::WRL::ComPtr<ID3D12Device9> {
            return _device;
        }

        auto GetDescriptorHeaps() -> std::vector<ID3D12DescriptorHeap*> {
            return {
                _srvHeaps->Heap()->Native().Get(),
                _samplerHeaps->Heap()->Native().Get()
            };
        }

    private:
        Microsoft::WRL::ComPtr<IDXGIAdapter1> _adapter;
        Microsoft::WRL::ComPtr<ID3D12Device9> _device;
        // ---- Resources ----
        std::unique_ptr<D3D12HeapManager> _rtvHeaps;
        std::unique_ptr<D3D12HeapManager> _srvHeaps;
        std::unique_ptr<D3D12HeapManager> _samplerHeaps;
        std::unique_ptr<D3D12HeapManager> _dsvHeaps;
        Microsoft::WRL::ComPtr<ID3D12RootSignature> _rootSignature;
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> _graphicsQueue;
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> _computeQueue;
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> _copyQueue;
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> _uploadQueue;

#if ENABLE_PROFILER
        tracy::D3D12QueueCtx* _tracyCtx;
#endif
        auto CreateCommandQueue(
            CommandListType type,
            std::string name
        ) -> Microsoft::WRL::ComPtr<ID3D12CommandQueue>;
    };
}
