#
#include "input/GameInputHandler.hxx"
#include <GameInput.h>
#include <Windows.h>
#include <iostream>
#include <tracy/Tracy.hpp>

namespace playground::input {
    void ReadingCallback(GameInputCallbackToken token, void* context, IGameInputReading* reading, bool overrun) {

    }

    GameInputHandler::GameInputHandler() {
        HRESULT hr = GameInputCreate(&_gameInput);
        if (FAILED(hr)) {
            exit(1);
        }

        auto result = _gameInput->GetCurrentReading(GameInputKindKeyboard | GameInputKindGamepad | GameInputKindMouse, nullptr, &_lastReading);

        if (result != S_OK) {
            exit(1);
        }
    }

    GameInputHandler::~GameInputHandler() {
    }

    eastl::vector<InputEvent, StackAllocator> GameInputHandler::PollEvents(StackAllocator& alloc) {
        ZoneScopedNC("GameInputHandler Poll", tracy::Color::Blue1);
        auto vec = eastl::vector<InputEvent, StackAllocator>(alloc);

        auto result = _gameInput->GetCurrentReading(GameInputKindKeyboard | GameInputKindGamepad | GameInputKindMouse, nullptr, &_nextReading);

        if (result != S_OK && result != E_NOTIMPL) {
            exit(1);
        }

        if (_nextReading == nullptr) {
            return vec;
        }

        double ts = _nextReading->GetTimestamp();

        _lastReading = _nextReading;

        return vec;
    }
}
