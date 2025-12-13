#pragma once

#include <cstdint>

namespace playground::input {
    enum class InputDevice : uint8_t {
        Mouse = 0,
        Keyboard = 1,
        Controller0,
        Controller1,
        Controller2,
        Controller3,
        Controller4,
        Controller5,
        Controller6,
        Controller7
    };
}
