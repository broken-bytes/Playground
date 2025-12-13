#pragma once

#include <cstdint>
#include <memory>
#include "rendering/RenderTarget.hxx"

namespace playground::rendering {
	class Swapchain {
	public:
		virtual ~Swapchain() = default;
		virtual auto Swap() -> void = 0;
        virtual auto BackBufferIndex() -> uint8_t = 0;
	};
}
