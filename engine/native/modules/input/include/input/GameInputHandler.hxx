#pragma once

#ifdef _WIN32
#include "input/IInputHandler.hxx"
#include <GameInput.h>
#include <wrl.h>

namespace playground::input {
    class GameInputHandler : public IInputHandler {
    public:
        GameInputHandler();
        ~GameInputHandler() override;
        eastl::vector<InputEvent, StackAllocator> PollEvents(StackAllocator& alloc) override;

    private:
        Microsoft::WRL::ComPtr<IGameInput> _gameInput;
        Microsoft::WRL::ComPtr<IGameInputReading> _lastReading;
        Microsoft::WRL::ComPtr<IGameInputReading> _nextReading;
        GameInputKeyState _lastKeyboard{};
        GameInputMouseState _lastMouse{};
    };
}
#endif
