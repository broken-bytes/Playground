#pragma once

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <wrl.h>
#include <directx/d3dx12.h>
#include "rendering/Texture.hxx"
#include "rendering/d3d12/D3D12CPUResourceHandle.hxx"
#include <EASTL/vector.h>
#include <shared/Arena.hxx>

namespace playground::rendering::d3d12 {
    using ArenaType = memory::VirtualArena;
    using Allocator = memory::ArenaAllocator<ArenaType>;

    class D3D12Texture : public Texture {
    public:
        D3D12Texture(
            Microsoft::WRL::ComPtr<ID3D12Device9> device,
            uint32_t width,
            uint32_t height,
            std::vector<std::vector<uint8_t>> mips,
            std::shared_ptr<D3D12ResourceHandle> handle,
            Allocator& alloc
        ) : _data(eastl::vector<eastl::vector<uint8_t, Allocator>, Allocator>(alloc)) {

            auto textureDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_BC3_UNORM, width, height, 1, mips.size(), 1, 0.5f, D3D12_RESOURCE_FLAG_NONE, D3D12_TEXTURE_LAYOUT_UNKNOWN);
            D3D12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
            auto result = device->CreateCommittedResource(
                &heapProps,
                D3D12_HEAP_FLAG_NONE,
                &textureDesc,
                D3D12_RESOURCE_STATE_COPY_DEST,  // Start in COPY_DEST state
                nullptr,
                IID_PPV_ARGS(&_texture)
            );            

            const UINT64 uploadBufferSize = GetRequiredIntermediateSize(_texture.Get(), 0, mips.size());

            uint32_t mipWidth = width;
            uint32_t mipHeight = height;

            for (int x = 0; x < mips.size(); x++) {
                auto& mip = mips[x];
                auto data = eastl::vector<uint8_t, Allocator>(alloc);
                data.resize(mip.size());
                memcpy(data.data(), mip.data(), mip.size());
                _data.emplace_back(std::move(data));

                UINT blockSize = 16;
                UINT rowBlocks = std::max(1u, (mipWidth + 3) / 4);
                UINT colBlocks = std::max(1u, (mipHeight + 3) / 4);

                D3D12_SUBRESOURCE_DATA subresource = {};
                subresource.pData = _data[x].data();
                subresource.RowPitch = rowBlocks * blockSize;
                subresource.SlicePitch = subresource.RowPitch * colBlocks;
                _subresources.push_back(subresource);

                mipWidth /= 2;
                mipHeight /= 2;
            }

            D3D12_HEAP_PROPERTIES uploadHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
            D3D12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);

            device->CreateCommittedResource(
                &uploadHeapProps,
                D3D12_HEAP_FLAG_NONE,
                &bufferDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&_stagingBuffer)
            );

            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srvDesc.Format = DXGI_FORMAT_BC3_UNORM;
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MostDetailedMip = 0;
            srvDesc.Texture2D.MipLevels = mips.size();
            srvDesc.Texture2D.PlaneSlice = 0;
            srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

            _handle = handle;

            device->CreateShaderResourceView(_texture.Get(), &srvDesc, handle->GetCPUHandle());
        }

        auto Texture() const -> Microsoft::WRL::ComPtr<ID3D12Resource> {
            return _texture;
        }

        auto StagingBuffer() const -> Microsoft::WRL::ComPtr<ID3D12Resource> {
            return _stagingBuffer;
        }

        auto TextureData() const -> const std::vector<D3D12_SUBRESOURCE_DATA>& {
            return _subresources;
        }

        auto Free() {
            _stagingBuffer = nullptr;
        }

        auto CPUHandle() -> CD3DX12_CPU_DESCRIPTOR_HANDLE {
            return _handle->GetCPUHandle();
        }

        auto GPUHandle() -> CD3DX12_GPU_DESCRIPTOR_HANDLE {
            return _handle->GetGPUHandle();
        }

        uint32_t ID() const override {
            return _handle->ID();
        }

    private:
        eastl::vector<eastl::vector<uint8_t, Allocator>, Allocator> _data;
        Microsoft::WRL::ComPtr<ID3D12Resource> _texture;
        Microsoft::WRL::ComPtr<ID3D12Resource> _stagingBuffer;
        std::vector<D3D12_SUBRESOURCE_DATA> _subresources;
        std::shared_ptr<D3D12ResourceHandle> _handle;
    };
}
