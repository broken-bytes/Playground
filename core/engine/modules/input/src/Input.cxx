#include "input/Input.hxx"
#include "input/InputAction.hxx"
#include "input/IInputHandler.hxx"
#include "input/ButtonState.hxx"
#ifdef _WIN32
#include "input/GameInputHandler.hxx"
#include "input/RawInputHandler.hxx"
#endif
#include <events/Events.hxx>
#include <events/Event.hxx>
#include <events/InputEvent.hxx>
#include <events/SystemEvent.hxx>
#include <stdexcept>
#include <iostream>
#include <SDL3/SDL.h>
#include <array>
#include<thread>
#include <vector>
#include <concurrentqueue.h>

namespace playground::input {
    struct Button {
        ButtonState state;
        uint64_t tick;
        double timestamp;
    };

    struct InputState {
        std::array<Button, 5> mouseButtons;
        std::array<Button, 256> keys;
        std::array<Button, 8> controllerButtons;
        std::array<float, 6> controllerAxes;
        bool mouseXMoved = false;
        bool mouseYMoved = false;
        float mouseX;
        float mouseY;
    };

    constexpr float deadZone = 0.05f;
    constexpr uint16_t tickRate = 1000000;
    uint64_t currentTick = 0;

    std::unique_ptr<IInputHandler> inputHandler;

#ifdef _WIN32
    std::unique_ptr<IInputHandler> rawInputHandler;
#endif

    StackArena pollArena;
    StackAllocator pollAllocator(&pollArena, "Input Poll Allocator");

    InputState currentInputState;

    moodycamel::ConcurrentQueue<InputAction> inputEventsQueue;

    void ProcessKeyboard(InputEvent& event);
    void ProcessController(InputEvent& event, uint8_t id);
    void ProcessMouse(InputEvent& event);
    void ProcessButtonDiff(Button* events, size_t count);
    void EmitButtonEvents(InputDevice device, Button* buttons, size_t count);

    auto Init(void* windowHandle) -> void {
#if _WIN32
        inputHandler = std::make_unique<GameInputHandler>();
        rawInputHandler = std::make_unique<RawInputHandler>(windowHandle);
#endif
	}

    auto Shutdown() -> void {
        inputHandler = nullptr;
    }

    auto Update() -> void {
        auto now = std::chrono::steady_clock::now();
        pollArena.Reset();

        //auto gamePadevents = inputHandler->PollEvents();
#ifdef _WIN32
        auto rawEvents = rawInputHandler->PollEvents(pollAllocator);
#endif
        // TODO: Do Input Logic Here
        for (auto& newInput : rawEvents) {
            switch (newInput.device) {
            case InputDevice::Keyboard:
                ProcessKeyboard(newInput);
                break;
            case InputDevice::Mouse:
                ProcessMouse(newInput);
                break;
            case InputDevice::Controller0:
                ProcessController(newInput, 0);
                break;
            }
        }

        // Check for all buttons if they were updated this frame or not and apply held/released
        ProcessButtonDiff(currentInputState.mouseButtons.data(), currentInputState.mouseButtons.size());
        ProcessButtonDiff(currentInputState.keys.data(), currentInputState.keys.size());
        ProcessButtonDiff(currentInputState.controllerButtons.data(), currentInputState.controllerButtons.size());

        // Now emit all chanegd buttons + all axis

        if (currentInputState.mouseXMoved) {
            inputEventsQueue.enqueue(InputAction{ .type = InputType::Axis, .device = InputDevice::Mouse, .axisAction = AxisAction {.axisId = 0, .value = currentInputState.mouseX } });
            currentInputState.mouseXMoved = false;
        }
        else {
            currentInputState.mouseX = 0.0f; // Reset mouse X if not moved
        }
        if (currentInputState.mouseYMoved) {
            inputEventsQueue.enqueue(InputAction{ .type = InputType::Axis, .device = InputDevice::Mouse, .axisAction = AxisAction {.axisId = 1, .value = currentInputState.mouseY } });
            currentInputState.mouseYMoved = false;
        }
        else {
            currentInputState.mouseY = 0.0f; // Reset mouse Y if not moved
        }

        EmitButtonEvents(InputDevice::Mouse, currentInputState.mouseButtons.data(), currentInputState.mouseButtons.size());
        EmitButtonEvents(InputDevice::Keyboard, currentInputState.keys.data(), currentInputState.keys.size());

        if (currentInputState.controllerAxes[0] > deadZone) {
            inputEventsQueue.enqueue(InputAction{ .type = InputType::Axis, .device = InputDevice::Controller0, .axisAction = AxisAction {.axisId = 0, .value = currentInputState.controllerAxes[0]} });
        }
        if (currentInputState.controllerAxes[1] > deadZone) {
            inputEventsQueue.enqueue(InputAction{ .type = InputType::Axis, .device = InputDevice::Controller0, .axisAction = AxisAction {.axisId = 1, .value = currentInputState.controllerAxes[1]} });
        }
        if (currentInputState.controllerAxes[2] > deadZone) {
            inputEventsQueue.enqueue(InputAction{ .type = InputType::Axis, .device = InputDevice::Controller0, .axisAction = AxisAction {.axisId = 2, .value = currentInputState.controllerAxes[2]} });
        }
        if (currentInputState.controllerAxes[3] > deadZone) {
            inputEventsQueue.enqueue(InputAction{ .type = InputType::Axis, .device = InputDevice::Controller0, .axisAction = AxisAction {.axisId = 3, .value = currentInputState.controllerAxes[3]} });
        }
        if (currentInputState.controllerAxes[4] > deadZone) {
            inputEventsQueue.enqueue(InputAction{ .type = InputType::Axis, .device = InputDevice::Controller0, .axisAction = AxisAction {.axisId = 4, .value = currentInputState.controllerAxes[4]} });
        }
        if (currentInputState.controllerAxes[5] > deadZone) {
            inputEventsQueue.enqueue(InputAction{ .type = InputType::Axis, .device = InputDevice::Controller0, .axisAction = AxisAction {.axisId = 5, .value = currentInputState.controllerAxes[5]} });
        }
        EmitButtonEvents(InputDevice::Controller0, currentInputState.controllerButtons.data(), currentInputState.controllerButtons.size());

        currentTick++;

        SDL_PumpEvents();

        auto end = std::chrono::steady_clock::now();
        auto elapsed = end - now;

        if (elapsed.count() < tickRate) {
            std::this_thread::sleep_for(std::chrono::nanoseconds(tickRate - elapsed.count()));
        }
	}

    auto FetchInput(InputAction& action) -> bool {
        return inputEventsQueue.try_dequeue(action);
    }

    void ProcessKeyboard(InputEvent& event) {
        currentInputState.keys[event.actionId].tick = currentTick;
        currentInputState.keys[event.actionId].timestamp = event.timestamp;
        switch (event.type) {
            case InputEventType::ButtonDown:
                currentInputState.keys[event.actionId].state = ButtonState::Down;
                break;
            case InputEventType::ButtonUp:
                currentInputState.keys[event.actionId].state = ButtonState::Up;
                break;
        }
    }

    void ProcessController(InputEvent& event, uint8_t id) {

    }

    void ProcessMouse(InputEvent& event) {
        switch (event.type) {
        case InputEventType::AxisMoved:
            if (event.actionId == 0) {
                currentInputState.mouseX += event.value;
                currentInputState.mouseXMoved = true;
            }
            else {
                currentInputState.mouseY += event.value;
                currentInputState.mouseYMoved = true;
            }
            break;
        case InputEventType::ButtonDown:
            currentInputState.mouseButtons[event.actionId].tick = currentTick;
            currentInputState.mouseButtons[event.actionId].timestamp = event.timestamp;
            currentInputState.mouseButtons[event.actionId].state = ButtonState::Down;
            break;
        case InputEventType::ButtonUp:
            currentInputState.mouseButtons[event.actionId].tick = currentTick;
            currentInputState.mouseButtons[event.actionId].timestamp = event.timestamp;
            currentInputState.mouseButtons[event.actionId].state = ButtonState::Up;
            break;
        }
    }

    void ProcessButtonDiff(Button* events, size_t count) {
        for (int x = 0; x < count; x++) {
            auto& current = events[x];
            if (current.tick != currentTick) {
                if (current.state == ButtonState::Down) {
                    current.state = ButtonState::Held;
                }
                else if (current.state == ButtonState::Up) {
                    current.state = ButtonState::Released;
                }
            }
        }
    }

    void EmitButtonEvents(InputDevice device, Button* buttons, size_t count) {
        for (uint16_t x = 0; x < count; x++) {
            if (buttons[x].state == ButtonState::Released) {
                continue;
            }
            inputEventsQueue.enqueue(InputAction{
                   .type = InputType::Button,
                   .device = device,
                   .buttonAction = ButtonAction {
                       .buttonId = x,
                       .state = buttons[x].state
                   }
            });
        }
    }
}
