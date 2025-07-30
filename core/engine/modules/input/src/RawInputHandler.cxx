#ifdef _WIN32
#include "input/RawInputHandler.hxx"
#include <shared/Logger.hxx>
#include <Windows.h>
#include <iostream>
#include <events/Events.hxx>
#include <events/SystemEvent.hxx>
#include <SDL3/SDL.h>
#include <functional>
#include <tracy/Tracy.hpp>

namespace playground::input {
    std::function<void(LPARAM)> procCallback;

    void EmitWindowFocusEvent(HWND handle) {
        events::WindowFocusEvent enterEvent;
        playground::events::Emit(&enterEvent);
        TRACKMOUSEEVENT tme = {
            sizeof(TRACKMOUSEEVENT),
            TME_LEAVE,
            handle,
            0
        };
        TrackMouseEvent(&tme);
    }

    void EmitWindowLostFocusEvent() {
        events::WindowLostFocusEvent leaveEvent;
        playground::events::Emit(&leaveEvent);
    }

    LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        switch (msg) {
        case WM_MOUSEMOVE:
            EmitWindowFocusEvent(hwnd);
            break;
        case WM_MOUSELEAVE:
            EmitWindowLostFocusEvent();
            break;
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

    RawInputHandler::RawInputHandler(void* windowHandle): _windowHandle(windowHandle) {
        logging::logger::Info("Initializing Raw Input Handler", "input");
        RAWINPUTDEVICE rid[2] = {};

        rid[0].usUsagePage = 0x01;
        rid[0].usUsage = 0x02;
        rid[0].dwFlags = 0;
        rid[0].hwndTarget = (HWND)windowHandle;

        rid[1].usUsagePage = 0x01;
        rid[1].usUsage = 0x06;
        rid[1].dwFlags = 0;
        rid[1].hwndTarget = (HWND)windowHandle;

        procCallback = [&](LPARAM lParam) { HandleRawInput(lParam); };

        LARGE_INTEGER freq;
        QueryPerformanceFrequency(&freq);

        _qpcToSeconds = 1.0 / double(freq.QuadPart);

        if (!RegisterRawInputDevices(rid, 2, sizeof(RAWINPUTDEVICE))) {
            logging::logger::Error("Failed to register raw input devices. Error: " + std::to_string(GetLastError()), "input");
            exit(1);
        }

        SetWindowLongPtr((HWND)windowHandle, GWLP_WNDPROC, (LONG_PTR)WindowProc);
    }

    RawInputHandler::~RawInputHandler() {
    }

    eastl::vector<InputEvent, StackAllocator> RawInputHandler::PollEvents(StackAllocator& alloc) {
        ZoneScopedNC("RawInputHandler Poll", tracy::Color::Blue1);
        auto data = eastl::vector<InputEvent, StackAllocator>(alloc);

        InputEvent event = {};
        while(_inputQueue.try_dequeue(event)) {
            data.push_back(event);
        }

        return data;
    }

    void RawInputHandler::HandleRawInput(LPARAM lParam) {
        ZoneScopedNC("RawInputHandler HandleRawInput", tracy::Color::Blue2);

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

            event.device = InputDevice::Keyboard;
            event.type = down ? InputEventType::ButtonDown : InputEventType::ButtonUp;
            event.actionId = kb.MakeCode;
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
