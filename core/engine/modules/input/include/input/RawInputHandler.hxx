#pragma once

#ifdef _WIN32
#include "input/IInputHandler.hxx"
#include <EASTL/array.h>
#include <EASTL/vector.h>
#include <atomic>
#include <Windows.h>
#include <concurrentqueue.h>

namespace playground::input {
    class RawInputHandler : public IInputHandler {
    public:
        RawInputHandler(void* windowHandle);
        ~RawInputHandler() override;
        eastl::vector<InputEvent, StackAllocator> PollEvents(StackAllocator& alloc) override;
    private:
        moodycamel::ConcurrentQueue<InputEvent> _inputQueue;
        double _qpcToSeconds;

        void HandleRawInput(LPARAM lParam);
    };
}
#endif
