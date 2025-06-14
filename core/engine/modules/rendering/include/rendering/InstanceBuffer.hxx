#pragma once

#include "rendering/Buffer.hxx"

#include <cstdint>

namespace playground::rendering {
    class InstanceBuffer : public Buffer {
    public:
        InstanceBuffer(size_t count) : Buffer(count) {};
        virtual ~InstanceBuffer() = default;
        virtual auto Id() const -> uint64_t = 0;
        virtual void SetData(const void* data, size_t size) = 0;
        virtual void Bind() const = 0;
    };
}
