#include "input/Input.hxx"
#include <events/Events.hxx>
#include <events/Event.hxx>
#include <events/InputEvent.hxx>
#include <events/SystemEvent.hxx>
#include <stdexcept>
#include "input/IInputHandler.hxx"
#ifdef _WIN32
#include "input/GameInputHandler.hxx"
#include "input/RawInputHandler.hxx"
#endif
#include <iostream>
#include <SDL3/SDL.h>

namespace playground::input {
    std::unique_ptr<IInputHandler> inputHandler;

#ifdef _WIN32
    std::unique_ptr<IInputHandler> rawInputHandler;
#endif

    StackArena lastFrameArena;
    StackArena pollArena;
    StackAllocator lastFrameAlloc(&lastFrameArena, "Input Last Frame Allocator");
    StackAllocator pollAllocator(&pollArena, "Input Poll Allocator");

    eastl::vector<InputEvent, StackAllocator> lastFrame(lastFrameAlloc);

    struct ButtonState {
        bool down;
        bool pressed;
        bool released;
        double timestamp;
    };

    struct InputState {
        ButtonState mouseButtons[5];
        ButtonState keys[256];
        ButtonState controllerButtons[8];
        float controllerAxes[6];
    };

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
        pollArena.Reset();

        //auto gamePadevents = inputHandler->PollEvents();
#ifdef _WIN32
        auto mouseEvents = rawInputHandler->PollEvents(pollAllocator);
#endif
        SDL_Event event;
        SDL_PumpEvents();

        // TODO: Do Input Logic Here
        lastFrame.clear();
        lastFrameArena.Reset();
        lastFrame.resize(mouseEvents.size());
        lastFrame.insert(lastFrame.end(), mouseEvents.begin(), mouseEvents.end());
	}
}
