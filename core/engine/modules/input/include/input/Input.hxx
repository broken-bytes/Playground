#pragma once

namespace playground::input {
	auto Init(void* window) -> void;
    auto Shutdown() -> void;
	auto Update() -> void;
}
