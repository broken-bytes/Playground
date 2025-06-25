#pragma once

#include <input/InputDevice.hxx>

namespace playground::inputmanager {
    bool WasKeyDown(const char* actionName);
    bool WasKeyUp(const char* actionName);
    bool WasKeyHeld(const char* actionName);
    bool WasButtonDown(const char* actionName);
    bool WasButtonUp(const char* actionName);
    bool WasButtonHeld(const char* actionName);
    float GetAxis(const char* actionName);

    void Init();
    void Update();

    float GetAxis(const char* name);
    bool IsButtonDown(const char* name);
    bool IsButtonUp(const char* name);
    bool IsButtonPressed(const char* name);

    void SetVirtualAxisPos(const char* axisName, uint16_t buttonId);
    void SetVirtualAxisNeg(const char* axisName, uint16_t buttonId);
    void SetPhysicalAxisPos(const char* axisName, input::InputDevice device, uint16_t axisId);
    void SetVPhysicalAxisNeg(const char* axisName, input::InputDevice device, uint16_t axisId);
    void SetButtonMapping(const char* buttonName, input::InputDevice deviceId, uint16_t buttonId);
}
