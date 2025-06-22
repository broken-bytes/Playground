#ifdef _WIN32
#include "input/RawInputHandler.hxx"
#include <Windows.h>
#include <iostream>
#include <events/Events.hxx>
#include <events/SystemEvent.hxx>
#include <SDL3/SDL.h>
#include <functional>

namespace playground::input {
    std::function<void(LPARAM)> procCallback;

    LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        switch (msg) {
        case WM_INPUT:
            procCallback(lParam);
            break;
        case WM_CLOSE:
            events::SystemEvent quitEvent(events::SystemEventType::Quit);
            playground::events::Emit(&quitEvent);
            PostQuitMessage(0);
            return 0;
        }

        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    RawInputHandler::RawInputHandler(void* windowHandle) {
        RAWINPUTDEVICE rid[2] = {};

        rid[0].usUsagePage = 0x01;
        rid[0].usUsage = 0x02;
        rid[0].dwFlags = RIDEV_INPUTSINK;
        rid[0].hwndTarget = (HWND)windowHandle;

        rid[1].usUsagePage = 0x01;
        rid[1].usUsage = 0x06;
        rid[1].dwFlags = RIDEV_INPUTSINK;
        rid[1].hwndTarget = (HWND)windowHandle;

        procCallback = [&](LPARAM lParam) { HandleRawInput(lParam); };

        LARGE_INTEGER freq;
        QueryPerformanceFrequency(&freq);

        _qpcToSeconds = 1.0 / double(freq.QuadPart);

        if (!RegisterRawInputDevices(rid, 2, sizeof(RAWINPUTDEVICE))) {
            exit(1);
        }

        SetWindowLongPtr((HWND)windowHandle, GWLP_WNDPROC, (LONG_PTR)WindowProc);
    }

    RawInputHandler::~RawInputHandler() {
    }

    eastl::vector<InputEvent, StackAllocator> RawInputHandler::PollEvents(StackAllocator& alloc) {
        auto data = eastl::vector<InputEvent, StackAllocator>(alloc);

        InputEvent event = {};
        while(_inputQueue.try_dequeue(event)) {
            data.push_back(event);
        }

        return data;
    }

    void RawInputHandler::HandleRawInput(LPARAM lParam) {
        UINT size;
        GetRawInputData((HRAWINPUT)lParam, RID_INPUT, nullptr, &size, sizeof(RAWINPUTHEADER));

        BYTE* buffer = new BYTE[size];
        if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, buffer, &size, sizeof(RAWINPUTHEADER)) != size) {
            delete[] buffer;
            return;
        }

        LARGE_INTEGER now;
        QueryPerformanceCounter(&now);

        RAWINPUT* raw = (RAWINPUT*)buffer;
        InputEvent event;
        event.timestamp = double(now.QuadPart) * _qpcToSeconds;

        if (raw->header.dwType == RIM_TYPEKEYBOARD) {
            RAWKEYBOARD& kb = raw->data.keyboard;
            bool down = !(kb.Flags & RI_KEY_BREAK);

            std::cout << "[Keyboard] Key " << kb.VKey << (down ? " down" : " up") << std::endl;
        }
        else if (raw->header.dwType == RIM_TYPEMOUSE) {
            RAWMOUSE& m = raw->data.mouse;
            event.device = InputDevice::Mouse;

            if (m.lLastX) {
                event.actionId = 0;
                event.value = m.lLastX;
                event.type = InputEventType::AxisMoved;
            }

            if (m.lLastY) {
                event.actionId = 1;
                event.value = m.lLastY;
                event.type = InputEventType::AxisMoved;
            }

            if (m.usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN) {
                event.actionId = 0;
                event.type = InputEventType::ButtonDown;
            }
            if (m.usButtonFlags & RI_MOUSE_LEFT_BUTTON_UP) {
                event.actionId = 0;
                event.type = InputEventType::ButtonDown;
            }

            if (m.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN) {
                event.actionId = 1;
                event.type = InputEventType::ButtonDown;
            }
            if (m.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_UP) {
                event.actionId = 1;
                event.type = InputEventType::ButtonDown;
            }

            if (m.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_DOWN) {
                event.actionId = 2;
                event.type = InputEventType::ButtonDown;
            }
            if (m.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_UP) {
                event.actionId = 2;
                event.type = InputEventType::ButtonDown;
            }

            if (m.usButtonFlags & RI_MOUSE_BUTTON_4_DOWN) {
                event.actionId = 3;
                event.type = InputEventType::ButtonDown;
            }
            if (m.usButtonFlags & RI_MOUSE_BUTTON_4_DOWN) {
                event.actionId = 3;
                event.type = InputEventType::ButtonDown;
            }

            if (m.usButtonFlags & RI_MOUSE_BUTTON_5_DOWN) {
                event.actionId = 4;
                event.type = InputEventType::ButtonDown;
            }
            if (m.usButtonFlags & RI_MOUSE_BUTTON_5_DOWN) {
                event.actionId = 4;
                event.type = InputEventType::ButtonDown;
            }
        }

        _inputQueue.enqueue(event);

        delete[] buffer;
    }
}
#endif
