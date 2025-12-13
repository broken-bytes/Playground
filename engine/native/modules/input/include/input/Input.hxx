#pragma once

#include <input/InputAction.hxx>
#include <concurrentqueue.h>

namespace playground::input {
	auto Init(void* window) -> void;
    auto Shutdown() -> void;
	auto Update() -> void;

    auto FetchInput(InputAction& event) -> bool;

#if EDITOR
    void SetCapturesInput(bool capture);
#endif
}
