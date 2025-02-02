#pragma once

namespace playground::rendering {
	class Buffer {
	public:
		Buffer(size_t size) : _size(size) {}
		virtual ~Buffer() = default;

		auto Size() const -> size_t { return _size; }

	private:
		size_t _size;
	};
}
