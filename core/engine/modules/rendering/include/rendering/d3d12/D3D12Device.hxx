#pragma once

#include <cstdint>
#include <directx/d3dx12.h>
#include <dxgi.h>
#include <dxgi1_4.h>
#include <dxgi1_5.h>
#include <wrl.h>

#include "rendering/Device.hxx"
#include "rendering/d3d12/D3D12HeapManager.hxx"

namespace playground::rendering::d3d12 {
    class D3D12Device : public rendering::Device, public std::enable_shared_from_this<D3D12Device> {
    public:
        explicit D3D12Device(void* window, uint8_t frameCount);
        ~D3D12Device();

        auto Flush() -> void override;
        auto CreateGraphicsContext(void* window, uint32_t width, uint32_t height) -> std::unique_ptr<Context> override;
        auto CreateUploadContext() -> std::unique_ptr<Context> override;
        auto CreateCommandList(
            CommandListType type,
            std::string name
        ) -> std::shared_ptr<CommandList> override;
        auto CreateFence() -> std::shared_ptr<Fence> override;
        auto CreateBuffer(uint64_t size) -> std::shared_ptr<Buffer> override;
        auto CreateRenderTarget(uint32_t width, uint32_t height, TextureFormat format) -> std::shared_ptr<RenderTarget> override;
        auto CreateDepthBuffer(uint32_t width, uint32_t height) -> std::shared_ptr<DepthBuffer> override;
        auto CreateMaterial(std::map<ShaderType, std::shared_ptr<Shader>> shaders,
            std::map<std::string, uint64_t> textures, std::map<std::string, float> floats,
            std::map<std::string, uint32_t> ints, std::map<std::string, bool> bools,
            std::map<std::string, std::array<float, 2>> vec2s, std::map<std::string, std::array<float, 3>> vec3s,
            std::map<std::string, std::array<float, 4>> vec4s) -> std::shared_ptr<Material> override;
        auto CompileShader(const std::string& shaderSource, ShaderType type) -> std::shared_ptr<Shader> override;
        auto CreateVertexBuffer(const void* data, uint64_t size) -> std::shared_ptr<VertexBuffer> override;
        auto UpdateVertexBuffer(std::shared_ptr<VertexBuffer> buffer, const void* data, uint64_t size) -> void override;
        auto CreateIndexBuffer(const uint32_t* indices, size_t len) -> std::shared_ptr<IndexBuffer> override;
        auto UpdateIndexBuffer(std::shared_ptr<IndexBuffer> buffer, std::vector<uint32_t> indices) -> void override;
        auto DestroyShader(uint64_t shaderHandle) -> void override;

    private:
        Microsoft::WRL::ComPtr<IDXGIFactory4> _factory;
        Microsoft::WRL::ComPtr<IDXGIAdapter1> _adapter;
        Microsoft::WRL::ComPtr<ID3D12Device9> _device;
        // ---- Resources ----
        std::unique_ptr<D3D12HeapManager> _rtvHeaps;
        std::unique_ptr<D3D12HeapManager> _srvHeaps;
        std::unique_ptr<D3D12HeapManager> _dsvHeaps;

        auto CreateCommandQueue(
            CommandListType type,
            std::string name
        ) -> Microsoft::WRL::ComPtr<ID3D12CommandQueue>;
    };
}
