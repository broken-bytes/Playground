#pragma once

#include "input/InputDevice.hxx"
#include <shared/Arena.hxx>
#include <EASTL/vector.h>
#include <cstdint>

namespace playground::input {
    enum class InputEventType {
        ButtonUp,
        ButtonDown,
        AxisMoved,
        PointerEnter,
        PointerLeave,
    };

    struct InputEvent {
        InputEventType type;
        InputDevice device;
        uint32_t actionId; // Keyboard: ScanCode | Mouse Axis: 0 = Mouse X, 1 Mouse Y | Controller Button : ScanCode | Controller Axis: 0 LSX, 1 LSY, 2 RSX, 3 RSY, 4 LT, 5 RT
        float value;
        double timestamp;
    };

    using StackArena = memory::StackArena<2 * 1024 * 1024>; // 2 MB Input buffer
    using StackAllocator = memory::ArenaAllocator<StackArena>;

    class IInputHandler {
    public:
        virtual ~IInputHandler() = default;
        virtual eastl::vector<InputEvent, StackAllocator> PollEvents(StackAllocator& alloc) = 0;
    };
}
