#pragma once

#include <cstdint>
#include "rendering/VertexBuffer.hxx"

namespace playground::rendering::d3d12 {
    class D3D12VertexBuffer : public rendering::VertexBuffer {
    public:
        explicit D3D12VertexBuffer(size_t size) : rendering::VertexBuffer(size) {

        };

        ~D3D12VertexBuffer() {

        }

        auto Id() const->uint64_t  {

        }

        void SetData(const void* data, size_t size) {

        }

        void Bind() const {

        }
    };
}
