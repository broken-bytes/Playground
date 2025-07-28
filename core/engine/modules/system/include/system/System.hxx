#pragma once

#include <cstdint>
#include <string>

namespace playground::system {
	auto Init(uint16_t width, uint16_t height, bool fullscreen, std::string name) -> void*;
}
