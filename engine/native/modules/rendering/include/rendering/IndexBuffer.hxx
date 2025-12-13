#pragma once

#include "Buffer.hxx"

#include <cstdint>

namespace playground::rendering {
	class IndexBuffer: public Buffer {
	public:
		IndexBuffer(size_t size) : Buffer(size) {}
		virtual ~IndexBuffer() = default;
		virtual auto Id() const-> uint64_t = 0;
		virtual void SetData(const void* data, size_t size) = 0;
		virtual void Bind() const = 0;
	};
}