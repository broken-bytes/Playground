#include <SDL3/SDL.h>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <pix.h>
#include <cassert>
#include "rendering/Context.hxx"
#include "rendering/d3d12/D3D12GraphicsContext.hxx"
#include "rendering/d3d12/D3D12GraphicsContext.hxx"
#include "rendering/d3d12/D3D12CommandAllocator.hxx"
#include "rendering/d3d12/D3D12CommandList.hxx"
#include "rendering/d3d12/D3D12DepthBuffer.hxx"
#include "rendering/d3d12/D3D12Material.hxx"
#include "rendering/d3d12/D3D12RenderTarget.hxx"
#include "rendering/d3d12/D3D12SwapChain.hxx"
#include "rendering/d3d12/D3D12IndexBuffer.hxx"
#include "rendering/d3d12/D3D12VertexBuffer.hxx"
#include "rendering/d3d12/D3D12Texture.hxx"
#include "rendering/PointLight.hxx"
#include "rendering/DirectionalLight.hxx"
#include <profiler/Profiler.hxx>
#include <tracy/Tracy.hpp>
#include <tracy/TracyD3D12.hpp>

namespace playground::rendering::d3d12
{
    D3D12GraphicsContext::D3D12GraphicsContext(
        std::string name,
        std::shared_ptr<D3D12Device> device,
        Microsoft::WRL::ComPtr<ID3D12RootSignature> opaqueRootSignature,
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> graphicsQueue,
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> transferQueue,
        void* window,
        uint32_t width,
        uint32_t height,
        bool isOffscreen
#if ENABLE_PROFILER
        , tracy::D3D12QueueCtx* ctx
#endif
    )
    {
        _device = device->GetDevice();
        _graphicsQueue = graphicsQueue;
        _transferQueue = transferQueue;
        _opaqueCommandList = std::static_pointer_cast<D3D12CommandList>(device->CreateCommandList(CommandListType::Graphics, name + "_OPAQUE_COMMAND_LIST"));
        _transparentCommandList = std::static_pointer_cast<D3D12CommandList>(device->CreateCommandList(CommandListType::Graphics, name + "_TRANSPARENT_COMMAND_LIST"));
        _shadowCommandList = std::static_pointer_cast<D3D12CommandList>(device->CreateCommandList(CommandListType::Graphics, name + "_SHADOW_COMMAND_LIST"));
        _transferCommandList = std::static_pointer_cast<D3D12CommandList>(device->CreateCommandList(CommandListType::Graphics, name + "_TRANSFER_COMMAND_LIST"));

        // Close command lists
        _opaqueCommandList->Close();
        _transparentCommandList->Close();
        _shadowCommandList->Close();
        _transferCommandList->Close();

        _device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence));
        _fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

        _opaqueRootSignature = opaqueRootSignature;

        _isOffscreen = isOffscreen;

        _mouseOverBuffer = std::make_unique<D3D12ReadbackBuffer>(_device, 1, 1);

        _cameraBuffer = std::static_pointer_cast<D3D12ConstantBuffer>(device->CreateConstantBuffer(nullptr, MAX_CAMERA_COUNT, sizeof(CameraBuffer), ConstantBuffer::BindingMode::RootCBV, name + "_CAMERA_BUFFER"));

        _pointLightsBuffer = std::static_pointer_cast<D3D12StructuredBuffer>(device->CreateStructuredBuffer(nullptr, MAX_POINT_LIGHTS, sizeof(PointLight), name + "_POINT_LIGHT_BUFFER"));

        _directionalLightBuffer = std::static_pointer_cast<D3D12ConstantBuffer>(device->CreateConstantBuffer(nullptr, 1, sizeof(DirectionalLight), ConstantBuffer::BindingMode::RootCBV, name + "_DIRECTIONAL_LIGHT_BUFFER"));

#if ENABLE_PROFILER
        _tracyCtx = ctx;
#endif
    }

    D3D12GraphicsContext::~D3D12GraphicsContext()
    {
        _opaqueCommandList->Close();
        _transparentCommandList->Close();
        _shadowCommandList->Close();
        _transferCommandList->Close();
    }

    auto D3D12GraphicsContext::Begin() -> void
    {
        PIXBeginEvent(PIX_COLOR_INDEX(0), "Begin Graphics Context");
        ZoneScopedN("RenderThread: GFX Begin");
        ZoneColor(tracy::Color::Orange3);
        // Wait until the GPU has finished execution
        if (_fenceValue != 0 && _fence->GetCompletedValue() < _fenceValue) {
            _fence->SetEventOnCompletion(_fenceValue, _fenceEvent);
            WaitForSingleObject(_fenceEvent, INFINITE);
        }
        // Reset all command lists so they can be in recording state
        _opaqueCommandList->Reset();
        _transparentCommandList->Reset();
        _shadowCommandList->Reset();
        _transferCommandList->Reset();

        _opaqueCommandList->Begin();
        _transparentCommandList->Begin();
        _shadowCommandList->Begin();
        _transferCommandList->Begin();
    }

    auto D3D12GraphicsContext::BeginRenderPass(RenderPass pass, std::shared_ptr<RenderTarget> colour, std::shared_ptr<DepthBuffer> depth) -> void {
        PIXBeginEvent(PIX_COLOR_INDEX((int)pass + 2), "Begin Opaque Pass");
        ZoneScopedN("RenderThread: Begin Opaque Pass");
        ZoneColor(tracy::Color::Orange2);
        assert(_currentPassList == nullptr && "A render pass was already started. Did you forget to end it?");

        D3D12_RENDER_PASS_RENDER_TARGET_DESC rtDesc = {};
        rtDesc.cpuDescriptor = std::static_pointer_cast<D3D12RenderTarget>(colour)->Handle();
        rtDesc.BeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR;
        rtDesc.BeginningAccess.Clear.ClearValue = D3D12_CLEAR_VALUE{
            .Format = DXGI_FORMAT_B8G8R8A8_UNORM,
            .Color = { 0.2f, 0.6f, 0.3f, 1 },
        };
        rtDesc.EndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;

        D3D12_RENDER_PASS_DEPTH_STENCIL_DESC dsDesc = {};
        dsDesc.cpuDescriptor = std::static_pointer_cast<D3D12DepthBuffer>(depth)->Handle();
        dsDesc.DepthBeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR;
        dsDesc.DepthBeginningAccess.Clear.ClearValue = D3D12_CLEAR_VALUE{
            .DepthStencil = D3D12_DEPTH_STENCIL_VALUE {
                .Depth = 1,
                .Stencil = 0,
            },
        };
        dsDesc.DepthEndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;
        dsDesc.StencilBeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_DISCARD;
        dsDesc.StencilEndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_DISCARD;

        switch (pass) {
        case RenderPass::Opaque:
            _currentPassList = _opaqueCommandList;
            _currentPassList->Native()->SetGraphicsRootSignature(_opaqueRootSignature.Get());
            _currentPassList->SetPrimitiveTopology(PrimitiveTopology::TRIANGLE_LIST);
            _currentPassList->BindConstantBuffer(_directionalLightBuffer, DIRECTIONAL_LIGHT_BUFFER_BINDING, 0);
            break;
        case RenderPass::Transparent:
            _currentPassList = _transparentCommandList;
            break;
        case RenderPass::Shadow:
            _currentPassList = _shadowCommandList;
            break;
        default:
            _currentPassList = _opaqueCommandList;
            break;
        }

        _currentPassList->Native()->BeginRenderPass(1, &rtDesc, &dsDesc, D3D12_RENDER_PASS_FLAG_NONE);
    }

    auto D3D12GraphicsContext::EndRenderPass() -> void {
        assert(_currentPassList != nullptr && "No render pass was started. Did you forget to start one?");
        ZoneScopedN("RenderThread: End Opaque Pass");
        ZoneColor(tracy::Color::Orange2);
        _currentPassList->Native()->EndRenderPass();
        _currentPassList = nullptr;
        PIXEndEvent();
    }

    auto D3D12GraphicsContext::Draw(uint32_t numIndices, uint32_t startIndex, uint32_t startVertex, uint32_t numInstances, uint32_t startInstance) -> void {
        _currentPassList->DrawIndexed(numIndices, startIndex, startVertex, numInstances, startInstance);
    }

    auto D3D12GraphicsContext::BindVertexBuffer(std::shared_ptr<VertexBuffer> buffer) -> void {
        _currentPassList->BindVertexBuffer(buffer, 0);
    }

    auto D3D12GraphicsContext::BindIndexBuffer(std::shared_ptr<IndexBuffer> buffer) -> void {
        _currentPassList->BindIndexBuffer(buffer);
    }

    auto D3D12GraphicsContext::BindInstanceBuffer(std::shared_ptr<InstanceBuffer> buffer) -> void {
        _currentPassList->BindInstanceBuffer(buffer);
    }

    auto D3D12GraphicsContext::BindCamera(uint8_t index) -> void {
        ZoneScopedN("RenderThread: Bind Camera");
        ZoneColor(tracy::Color::Orange3);
        assert(_cameraBuffer != nullptr && "Camera buffer is not initialized. Did you forget to call SetCameraData?");
        _currentPassList->BindConstantBuffer(_cameraBuffer, CAMERA_BUFFER_BINDING, index);
    }

    auto D3D12GraphicsContext::SetCameraData(std::array<CameraBuffer, MAX_CAMERA_COUNT>& cameras) -> void {
        ZoneScopedN("RenderThread: Set Camera Data");
        ZoneColor(tracy::Color::Orange3);
        _cameraBuffer->SetData(reinterpret_cast<void*>(cameras.data()), MAX_CAMERA_COUNT, 0);
    }

    auto D3D12GraphicsContext::BindMaterial(std::shared_ptr<Material> material) -> void {
        auto list = _currentPassList->Native();
        auto dxMat = std::static_pointer_cast<D3D12Material>(material);
        list->SetPipelineState(dxMat->pso.Get());
    }

    auto D3D12GraphicsContext::Finish() -> void
    {
        ZoneScopedN("RenderThread: GFX Finish");
        ZoneColor(tracy::Color::Orange3);
        _opaqueCommandList->Close();
        _transparentCommandList->Close();
        _shadowCommandList->Close();
        _transferCommandList->Close();

        std::vector<ID3D12CommandList*> lists;
        lists.push_back(_opaqueCommandList->Native().Get());
        lists.push_back(_transparentCommandList->Native().Get());
        lists.push_back(_shadowCommandList->Native().Get());
        lists.push_back(_transferCommandList->Native().Get());

        _graphicsQueue->ExecuteCommandLists(lists.size(), lists.data());

        _fenceValue++;
        _graphicsQueue->Signal(_fence.Get(), _fenceValue);
        
        PIXEndEvent();
    }

    auto D3D12GraphicsContext::TransitionIndexBuffer(std::shared_ptr<IndexBuffer> buffer) -> void {
        // Transition GPU buffer to index buffer state
        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            std::static_pointer_cast<D3D12IndexBuffer>(buffer)->Buffer().Get(),
            D3D12_RESOURCE_STATE_COPY_DEST,
            D3D12_RESOURCE_STATE_INDEX_BUFFER
        );

        _transferCommandList->Native()->ResourceBarrier(1, &barrier);
    }

    auto D3D12GraphicsContext::TransitionVertexBuffer(std::shared_ptr<VertexBuffer> buffer) -> void {
        // Transition GPU buffer to vertex buffer state
        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            std::static_pointer_cast<D3D12VertexBuffer>(buffer)->Buffer().Get(),
            D3D12_RESOURCE_STATE_COPY_DEST,
            D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER
        );

        _transferCommandList->Native()->ResourceBarrier(1, &barrier);
    }

    auto D3D12GraphicsContext::TransitionTexture(std::shared_ptr<Texture> texture) -> void
    {
        // Transition GPU buffer to texture state
        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            std::static_pointer_cast<D3D12Texture>(texture)->Texture().Get(),
            D3D12_RESOURCE_STATE_COPY_DEST,
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
        );

        _transferCommandList->Native()->ResourceBarrier(1, &barrier);
    }

    auto D3D12GraphicsContext::CopyToSwapchainBackBuffer(std::shared_ptr<RenderTarget> source, std::shared_ptr<Swapchain> swapchain) -> void {
        Microsoft::WRL::ComPtr<ID3D12Resource> backBuffer = std::static_pointer_cast<D3D12SwapChain>(swapchain)->GetCurrentBackBuffer();

        auto copyFromBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
            std::static_pointer_cast<D3D12RenderTarget>(source)->Resource().Get(),
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            D3D12_RESOURCE_STATE_COPY_SOURCE
        );

        _transferCommandList->Native()->ResourceBarrier(1, &copyFromBarrier);

        auto copyBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
            backBuffer.Get(),
            D3D12_RESOURCE_STATE_PRESENT,
            D3D12_RESOURCE_STATE_COPY_DEST
        );
        _transferCommandList->Native()->ResourceBarrier(1, &copyBarrier);
        

        // Copy the render target to the swap chain's back buffer
        _transferCommandList->Native()->CopyResource(backBuffer.Get(), std::static_pointer_cast<D3D12RenderTarget>(source)->Resource().Get());

        auto presentBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
            backBuffer.Get(),
            D3D12_RESOURCE_STATE_COPY_DEST,
            D3D12_RESOURCE_STATE_PRESENT
        );

        // Transition the back buffer back to PRESENT state
        _transferCommandList->Native()->ResourceBarrier(1, &presentBarrier);

        auto renderTargetBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
            std::static_pointer_cast<D3D12RenderTarget>(source)->Resource().Get(),
            D3D12_RESOURCE_STATE_COPY_SOURCE,
            D3D12_RESOURCE_STATE_RENDER_TARGET
        );

        _transferCommandList->Native()->ResourceBarrier(1, &renderTargetBarrier);
    }

    auto D3D12GraphicsContext::CopyToReadbackBuffer(std::shared_ptr<RenderTarget> source, std::shared_ptr<ReadbackBuffer> readbackBuffer) -> void {
        Microsoft::WRL::ComPtr<ID3D12Resource> backBuffer = std::static_pointer_cast<D3D12RenderTarget>(source)->Resource();

        auto copyFromBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
            std::static_pointer_cast<D3D12RenderTarget>(source)->Resource().Get(),
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            D3D12_RESOURCE_STATE_COPY_SOURCE
        );

        _transferCommandList->Native()->ResourceBarrier(1, &copyFromBarrier);
       
        auto d3d312RenderTarget = std::static_pointer_cast<D3D12RenderTarget>(source)->Resource().Get();
        D3D12_RESOURCE_DESC textureDesc = d3d312RenderTarget->GetDesc();
        UINT64 totalBytes = 0;
        D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint = {};
        UINT numRows = 0;
        UINT64 rowSizeInBytes = 0;
        D3D12_SUBRESOURCE_FOOTPRINT subresourceFootprint = {};
        _device->GetCopyableFootprints(&textureDesc, 0, 1, 0, &footprint, &numRows, &rowSizeInBytes, &totalBytes);

        D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
        srcLocation.pResource = d3d312RenderTarget;
        srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        srcLocation.SubresourceIndex = 0;

        D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
        dstLocation.pResource = std::static_pointer_cast<D3D12ReadbackBuffer>(readbackBuffer)->Buffer().Get();
        dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
        dstLocation.PlacedFootprint = footprint;

        _transferCommandList->Native()->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr);
        
        auto renderTargetBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
            std::static_pointer_cast<D3D12RenderTarget>(source)->Resource().Get(),
            D3D12_RESOURCE_STATE_COPY_SOURCE,
            D3D12_RESOURCE_STATE_RENDER_TARGET
        );

        _transferCommandList->Native()->ResourceBarrier(1, &renderTargetBarrier);
    }

    auto D3D12GraphicsContext::SetDirectionalLight(
        DirectionalLight& light
    ) -> void {
        ZoneScopedN("RenderThread: Set Directional Light");
        ZoneColor(tracy::Color::Orange3);
        assert(_directionalLightBuffer != nullptr && "Directional light buffer is not initialized. Did you forget to call SetDirectionalLightData?");
        _directionalLightBuffer->SetData(reinterpret_cast<void*>(&light), 1, 0);
    }

    auto D3D12GraphicsContext::SetViewport(
        uint32_t startX,
        uint32_t startY,
        uint32_t width,
        uint32_t height,
        uint32_t depthStart,
        uint32_t depthEnd
    ) -> void {
        _currentPassList->SetViewport(
            startX,
            startY,
            width,
            height,
            depthStart,
            depthEnd
        );
    }

    auto D3D12GraphicsContext::SetScissor(
        uint32_t left,
        uint32_t top,
        uint32_t right,
        uint32_t bottom
    ) -> void {
        _currentPassList->SetScissorRect(
            left,
            top,
            right,
            bottom
        );
    }

    auto D3D12GraphicsContext::MouseOverID() -> uint64_t
    {
        uint64_t* data = nullptr;
        size_t numRead = 0;
        _mouseOverBuffer->Read(reinterpret_cast<void**>(&data), &numRead);

        return *data;
    }
}
