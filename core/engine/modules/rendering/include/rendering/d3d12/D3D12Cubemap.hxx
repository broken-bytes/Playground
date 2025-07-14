#pragma once

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <wrl.h>
#include <directx/d3dx12.h>
#include "rendering/Cubemap.hxx"
#include "rendering/d3d12/D3D12CPUResourceHandle.hxx"
#include <EASTL/fixed_vector.h>
#include <EASTL/vector.h>
#include <shared/Arena.hxx>

namespace playground::rendering::d3d12 {
    using ArenaType = memory::VirtualArena;
    using Allocator = memory::ArenaAllocator<ArenaType>;

    class D3D12Cubemap : public Cubemap {
    public:
        D3D12Cubemap(
            Microsoft::WRL::ComPtr<ID3D12Device9> device,
            uint32_t width,
            uint32_t height,
            std::vector<std::vector<std::vector<uint8_t>>> faces,
            std::shared_ptr<D3D12ResourceHandle> handle,
            Allocator& alloc
        ) : _data(eastl::fixed_vector<eastl::vector<eastl::vector<uint8_t, Allocator>, Allocator>, 6, false, Allocator>(alloc)) {

            assert(faces.size() == 6 && "Cubemap must have 6 faces");
            assert(!faces.empty() && !faces.front().empty() && "Faces or mips missing");

            auto textureDesc = CD3DX12_RESOURCE_DESC::Tex2D(
                DXGI_FORMAT_BC3_UNORM,
                width,
                height,
                6,
                faces.front().size(),
                1,
                0.5f,
                D3D12_RESOURCE_FLAG_NONE,
                D3D12_TEXTURE_LAYOUT_UNKNOWN
            );
            D3D12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
            auto result = device->CreateCommittedResource(
                &heapProps,
                D3D12_HEAP_FLAG_NONE,
                &textureDesc,
                D3D12_RESOURCE_STATE_COPY_DEST,  // Start in COPY_DEST state
                nullptr,
                IID_PPV_ARGS(&_cubemap)
            );

            assert(SUCCEEDED(result) && "Failed to create cubemap resource");

            auto numSubresources = faces.size() * faces.front().size();

            const UINT64 uploadBufferSize = GetRequiredIntermediateSize(_cubemap.Get(), 0, numSubresources);

            uint32_t mipWidth = width;
            uint32_t mipHeight = height;

            _data.resize(6, eastl::vector<eastl::vector<uint8_t, Allocator>, Allocator>(alloc));

            uint32_t subresourceIndex = 0;
            for (uint32_t face = 0; face < 6; ++face) {
                _data[face] = eastl::vector<eastl::vector<uint8_t, Allocator>, Allocator>(alloc);

                mipWidth = width;
                mipHeight = height;

                for (uint32_t mip = 0; mip < faces[face].size(); ++mip) {
                    auto& mipLevel = faces[face][mip];
                    auto data = eastl::vector<uint8_t, Allocator>(alloc);
                    data.resize(mipLevel.size());
                    memcpy(data.data(), mipLevel.data(), mipLevel.size());
                    _data[face].emplace_back(std::move(data));

                    UINT blockSize = 16;
                    UINT rowBlocks = std::max(1u, (mipWidth + 3) / 4);
                    UINT colBlocks = std::max(1u, (mipHeight + 3) / 4);

                    D3D12_SUBRESOURCE_DATA subresource = {};
                    subresource.pData = _data[face][mip].data();
                    subresource.RowPitch = rowBlocks * blockSize;
                    subresource.SlicePitch = subresource.RowPitch * colBlocks;
                    _subresources.push_back(subresource);

                    ++subresourceIndex;

                    mipWidth /= 2;
                    mipHeight /= 2;
                }
            }

            D3D12_HEAP_PROPERTIES uploadHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
            D3D12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);

           result = device->CreateCommittedResource(
                &uploadHeapProps,
                D3D12_HEAP_FLAG_NONE,
                &bufferDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&_stagingBuffer)
            );

           assert(SUCCEEDED(result) && "Failed to create staging buffer for cubemap");

            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srvDesc.Format = DXGI_FORMAT_BC3_UNORM;
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
            srvDesc.TextureCube.MostDetailedMip = 0;
            srvDesc.TextureCube.MipLevels = faces.front().size();
            srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;

            _handle = handle;

            device->CreateShaderResourceView(_cubemap.Get(), &srvDesc, handle->GetCPUHandle());
        }

        auto Cubemap() const -> Microsoft::WRL::ComPtr<ID3D12Resource> {
            return _cubemap;
        }

        auto StagingBuffer() const -> Microsoft::WRL::ComPtr<ID3D12Resource> {
            return _stagingBuffer;
        }

        auto CubemapData() const -> const std::vector<D3D12_SUBRESOURCE_DATA>& {
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
        eastl::fixed_vector<eastl::vector<eastl::vector<uint8_t, Allocator>, Allocator>, 6, false, Allocator> _data;
        Microsoft::WRL::ComPtr<ID3D12Resource> _cubemap;
        Microsoft::WRL::ComPtr<ID3D12Resource> _stagingBuffer;
        std::vector<D3D12_SUBRESOURCE_DATA> _subresources;
        std::shared_ptr<D3D12ResourceHandle> _handle;
    };
}
