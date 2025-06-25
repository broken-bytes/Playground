#pragma once

#include "input/ButtonState.hxx"
#include "input/InputDevice.hxx"
#include <cstdint>

namespace playground::input {
    enum class InputType {
        Button,
        Axis
    };

    struct ButtonAction {
        uint16_t buttonId;
        ButtonState state;
    };

    struct AxisAction {
        int axisId;
        float value;
    };

    struct InputAction {
        InputType type;
        InputDevice device;

        union {
            ButtonAction buttonAction;
            AxisAction axisAction;
        };
    };
}
