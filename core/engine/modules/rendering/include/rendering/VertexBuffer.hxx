#pragma once

#include "rendering/Buffer.hxx"

#include <cstdint>

namespace playground::rendering {
	class VertexBuffer: public Buffer {
	public:
		VertexBuffer(size_t size) : Buffer(size) {};
		virtual ~VertexBuffer() = default;
		virtual auto Id() const->uint64_t = 0;
		virtual void SetData(const void* data, size_t size) = 0;
		virtual void Bind() const = 0;
	};
}
