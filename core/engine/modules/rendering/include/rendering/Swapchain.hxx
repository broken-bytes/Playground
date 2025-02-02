#pragma once

namespace playground::rendering {
	class Swapchain {
	public:
		virtual ~Swapchain() = default;
		virtual auto Swap() -> void = 0;
	};
}
