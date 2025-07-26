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
#include <shared/Job.hxx>
#include <shared/JobHandle.hxx>
#include <shared/JobSystem.hxx>
#include <stdexcept>
#include <iostream>
#include <SDL3/SDL.h>
#include <array>
#include<thread>
#include <vector>
#include <concurrentqueue.h>
#include <tracy/Tracy.hpp>

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
    constexpr float mouseSensitivity = 0.1f;
    uint64_t currentTick = 0;

    std::unique_ptr<IInputHandler> inputHandler;

#ifdef _WIN32
    std::unique_ptr<IInputHandler> rawInputHandler;
#endif

    StackArena pollArena;
    StackAllocator pollAllocator(&pollArena, "Input Poll Allocator");

    InputState currentInputState;

    moodycamel::ConcurrentQueue<InputAction> inputEventsQueue;

    void ProcessKeyboard(const eastl::vector<InputEvent, StackAllocator>& events);
    void ProcessController(const eastl::vector<InputEvent, StackAllocator>& events, uint8_t id);
    void ProcessMouse(const eastl::vector<InputEvent, StackAllocator>& events);
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
        ZoneScopedNC("Input: Update", tracy::Color::Blue1);
        pollArena.Reset();

        currentInputState.mouseX = 0.0f;
        currentInputState.mouseY = 0.0f;
        currentInputState.mouseXMoved = false;
        currentInputState.mouseYMoved = false;

        //auto gamePadevents = inputHandler->PollEvents();
#ifdef _WIN32
        auto rawEvents = rawInputHandler->PollEvents(pollAllocator);
#endif
        auto mouseJob = jobsystem::Job{
            .Name = "Process Mouse Input",
            .Priority = jobsystem::JobPriority::High,
            .Color = tracy::Color::Blue1,
            .Task = [rawEvents](uint8_t workerId) {
                ProcessMouse(rawEvents);
            }
        };

        auto keyboardJob = jobsystem::Job{
            .Name = "Process Keyboard Input",
            .Priority = jobsystem::JobPriority::High,
            .Color = tracy::Color::Blue2,
            .Task = [rawEvents](uint8_t workerId) {
                ProcessKeyboard(rawEvents);
            }
        };

        auto controllerJob = jobsystem::Job{
            .Name = "Process Controller Input",
            .Priority = jobsystem::JobPriority::High,
            .Color = tracy::Color::Blue3,
            .Task = [rawEvents](uint8_t workerId) {
                ProcessController(rawEvents, 0); // Assuming single controller for now
            }
        };

        auto completionJob = jobsystem::Job{
            .Name = "Process Input Completion",
            .Priority = jobsystem::JobPriority::High,
            .Color = tracy::Color::Blue4,
            .Dependencies = { mouseJob, keyboardJob, controllerJob },
            .Task = [](uint8_t workerId) {
                // This job is just to ensure all input processing is done before emitting events
            }
        };

        // Submit jobs to the job system
        auto completionHandle = jobsystem::Submit(completionJob);
        completionHandle->Wait();

        {
            ZoneScopedNC("Input: Process Button Diff", tracy::Color::Blue2);
            // Check for all buttons if they were updated this frame or not and apply held/released
            ProcessButtonDiff(currentInputState.mouseButtons.data(), currentInputState.mouseButtons.size());
            ProcessButtonDiff(currentInputState.keys.data(), currentInputState.keys.size());
            ProcessButtonDiff(currentInputState.controllerButtons.data(), currentInputState.controllerButtons.size());
        }
        // Now emit all chanegd buttons + all axis

        {
            ZoneScopedNC("Input: Emit Events", tracy::Color::Blue3);
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
        }

        currentTick++;

        SDL_PumpEvents();
	}

    auto FetchInput(InputAction& action) -> bool {
        return inputEventsQueue.try_dequeue(action);
    }

    void ProcessKeyboard(const eastl::vector<InputEvent, StackAllocator>& events) {
        for (auto& event : events) {
            ZoneScopedNC("Input: Process Keyboard", tracy::Color::Blue2);
            if (event.device != InputDevice::Keyboard) {
                continue; // Skip if not a keyboard event
            }

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
    }

    void ProcessController(const eastl::vector<InputEvent, StackAllocator>& events, uint8_t id) {
        ZoneScopedNC("Input: Process Controller", tracy::Color::Blue3);
    }

    void ProcessMouse(const eastl::vector<InputEvent, StackAllocator>& events) {
        for (auto& event : events) {
            ZoneScopedNC("Input: Process Mouse", tracy::Color::Blue4);
            if (event.device != InputDevice::Mouse) {
                continue; // Skip if not a mouse event
            }

            switch (event.type) {
            case InputEventType::AxisMoved:
                if (event.actionId == 0) {
                    currentInputState.mouseX += event.value * mouseSensitivity;
                    currentInputState.mouseXMoved = true;
                }
                else {
                    currentInputState.mouseY += event.value * mouseSensitivity;
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
    }

    void ProcessButtonDiff(Button* events, size_t count) {
        ZoneScopedNC("Input: Process Button Diff", tracy::Color::Blue4);
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
        ZoneScopedNC("Input: Emit Button Events", tracy::Color::Blue4);
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
