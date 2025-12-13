#pragma once

#include <stdexcept>
#include <wrl.h>
#include <directx/d3dx12.h>
#include "rendering/ReadbackBuffer.hxx"

namespace playground::rendering::d3d12 {
    class D3D12ReadbackBuffer : public ReadbackBuffer {
    public:
        D3D12ReadbackBuffer(
            Microsoft::WRL::ComPtr<ID3D12Device> device,
            uint32_t width,
            uint32_t height
        ) {
            _numBytes = width * height * 4;
            _cpuData = new uint8_t[_numBytes];
            // Create staging buffer for CPU read access
            D3D12_RESOURCE_DESC stagingDesc = {};
            stagingDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
            stagingDesc.Width = _numBytes;
            stagingDesc.Height = 1;
            stagingDesc.DepthOrArraySize = 1;
            stagingDesc.MipLevels = 1;
            stagingDesc.Format = DXGI_FORMAT_UNKNOWN;
            stagingDesc.SampleDesc.Count = 1;
            stagingDesc.SampleDesc.Quality = 0;
            stagingDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
            stagingDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

            auto stagingBuffer = nullptr;
            auto handle = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK);
            if (FAILED(device->CreateCommittedResource(
                &handle,
                D3D12_HEAP_FLAG_NONE,
                &stagingDesc,
                D3D12_RESOURCE_STATE_COPY_DEST,  // State before copy
                nullptr,  // No clear value for buffer
                IID_PPV_ARGS(&_stagingBuffer)
            ))) {
                throw std::runtime_error("Failed to create staging buffer");
            }
        }

        virtual ~D3D12ReadbackBuffer() {
            delete[] _cpuData;
        }

        void Read(void* result, size_t* numRead) override {
            *numRead = _numBytes;
            if (result == nullptr) {
                return;
            }
            _stagingBuffer->Map(0, nullptr, (reinterpret_cast<void**>(&_cpuData)));
            std::memcpy(result, _cpuData, *numRead);
            _stagingBuffer->Unmap(0, nullptr);
        }

        auto Buffer() const -> Microsoft::WRL::ComPtr<ID3D12Resource> {
            return _stagingBuffer;
        }

    private:
        Microsoft::WRL::ComPtr<ID3D12Resource> _stagingBuffer = nullptr;
        uint8_t* _cpuData = nullptr;
        size_t _numBytes = 0;
    };
}
