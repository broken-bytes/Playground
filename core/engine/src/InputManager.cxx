#include "playground/InputManager.hxx"
#include "playground/ini.h"
#include <input/InputDevice.hxx>
#include <input/Input.hxx>
#include <shared/Hasher.hxx>
#include <shared/Arena.hxx>
#include <EASTL/unordered_map.h>
#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

namespace playground::inputmanager {
    struct PhysicalButton {
        input::InputDevice device;
        uint16_t rawInputId;
    };

    struct PhysicalAxis {
        input::InputDevice device;
        uint16_t axisId;
    };

    // Digital keyboard buttons to axis
    struct VirtualAxis {
        uint16_t positiveInputId;
        uint16_t negativeInputId;
    };

    struct AxisMapping {
        uint64_t nameHash;
        std::optional<VirtualAxis> virtualAxis;
        std::optional<std::vector<PhysicalAxis>> physicalAxes;
    };

    struct ButtonMapping {
        uint64_t nameHash;
        std::vector<PhysicalButton> physicalButtons;
    };

    using StackArena = memory::StackArena<4 * 1024>; // 4KB arena for input manager
    using StackAllocator = memory::ArenaAllocator<StackArena>;

    StackArena inputManagerArena;
    StackAllocator inputManagerAllocator(&inputManagerArena, "Input Manager Allocator");

    std::vector<AxisMapping> axisMappings;
    std::vector<ButtonMapping> buttonMappings;

    struct ButtonState {
        bool isPressed;
        bool isDown;
        bool isUp;
    };

    using AxisMap = eastl::unordered_map<uint64_t, float, eastl::hash<uint64_t>, eastl::equal_to<uint64_t>, StackAllocator>;
    AxisMap axisValues = AxisMap({}, inputManagerAllocator);

    using ButtonMap = eastl::unordered_map<uint64_t, ButtonState, eastl::hash<uint64_t>, eastl::equal_to<uint64_t>, StackAllocator>;
    ButtonMap buttonStates = ButtonMap({}, inputManagerAllocator);

    std::optional<input::InputAction> unknownAction;

    bool LoadInputMappings(const std::string& path,
        std::vector<AxisMapping>& axes,
        std::vector<ButtonMapping>& buttons);

    void Init() {
        LoadInputMappings(std::filesystem::current_path().append("input.ini").string(), axisMappings, buttonMappings);
    }

    void Update() {
        inputManagerArena.Reset();

        axisValues = AxisMap({}, inputManagerAllocator);
        buttonStates = ButtonMap({}, inputManagerAllocator);

        auto inputAction = playground::input::InputAction();
        while (playground::input::FetchInput(inputAction)) {
            if (inputAction.type == input::InputType::Button) {
                bool consumed = false;
                for (auto& element : axisMappings) {
                    if (element.virtualAxis.has_value() && element.virtualAxis.value().positiveInputId == inputAction.buttonAction.buttonId) {
                        axisValues[element.nameHash] = 1;
                        consumed = true;
                    } else if (element.virtualAxis.has_value() && element.virtualAxis.value().negativeInputId == inputAction.buttonAction.buttonId) {
                        axisValues[element.nameHash] = -1;
                        consumed = true;
                    }
                }

                if (consumed) {
                    continue;
                }

                for (auto& element : buttonMappings) {
                    auto btn = std::find_if(element.physicalButtons.begin(), element.physicalButtons.end(), [inputAction](PhysicalButton& bnt) {
                        return bnt.rawInputId == inputAction.buttonAction.buttonId && bnt.device == inputAction.device;
                        });

                    if (btn != element.physicalButtons.end()) {
                        buttonStates[element.nameHash].isDown = inputAction.buttonAction.state == input::ButtonState::Down;
                        buttonStates[element.nameHash].isPressed = inputAction.buttonAction.state == input::ButtonState::Held;
                        buttonStates[element.nameHash].isUp = inputAction.buttonAction.state == input::ButtonState::Up;
                    }
                }
            }
            else {
                for (auto& element : axisMappings) {
                    if (!element.physicalAxes.has_value()) {
                        continue;
                    }
                    auto axis = std::find_if(element.physicalAxes.value().begin(), element.physicalAxes.value().end(), [element, inputAction](PhysicalAxis& axis) {
                        return axis.axisId == inputAction.axisAction.axisId && axis.device == inputAction.device;
                    });

                    if (axis != element.physicalAxes.value().end()) {
                        axisValues[element.nameHash] = inputAction.axisAction.value;
                    }
                }
            }
        }
    }

    float GetAxis(const char* name) {
        uint64_t nameHash = shared::Hash(name);
        auto axis = std::find_if(axisValues.begin(), axisValues.end(), [nameHash](auto& item) { return nameHash == item.first; });

        if (axis != axisValues.end()) {
            return axis->second;
        }

        return 0;
    }

    bool IsButtonDown(const char* name) {
        auto btn = std::find_if(buttonStates.begin(), buttonStates.end(), [name](auto& item) { return shared::Hash(name) == item.first; });

        if (btn != buttonStates.end()) {
            return btn->second.isDown;
        }

        return false;
    }

    bool IsButtonUp(const char* name) {
        auto btn = std::find_if(buttonStates.begin(), buttonStates.end(), [name](auto& item) { return shared::Hash(name) == item.first; });

        if (btn != buttonStates.end()) {
            return btn->second.isUp;
        }

        return false;
    }

    bool IsButtonPressed(const char* name) {
        auto btn = std::find_if(buttonStates.begin(), buttonStates.end(), [name](auto& item) { return shared::Hash(name) == item.first; });

        if (btn != buttonStates.end()) {
            return btn->second.isPressed;
        }

        return false;
    }

    void SetVirtualAxisPos(const char* axisName, uint16_t buttonId) {
        auto item = std::find_if(axisMappings.begin(), axisMappings.end(), [axisName](auto& elem) { return elem.nameHash == shared::Hash(axisName); });

        if (item->virtualAxis.has_value() && item != axisMappings.end()) {
            item->virtualAxis.value().positiveInputId = buttonId;
        }
        else {
            axisMappings.push_back(AxisMapping{ .nameHash = shared::Hash(axisName), .virtualAxis = VirtualAxis{.positiveInputId = buttonId, .negativeInputId = 0 }, .physicalAxes = {} });
        }
    }

    void SetVirtualAxisNeg(const char* axisName, uint16_t buttonId) {
        auto item = std::find_if(axisMappings.begin(), axisMappings.end(), [axisName](auto& elem) { return elem.nameHash == shared::Hash(axisName); });

        if (item->virtualAxis.has_value() && item != axisMappings.end()) {
            item->virtualAxis.value().negativeInputId = buttonId;
        }
        else {
            axisMappings.push_back(AxisMapping{ .nameHash = shared::Hash(axisName), .virtualAxis = VirtualAxis{.positiveInputId = 0, .negativeInputId = buttonId }, .physicalAxes = {} });
        }
    }

    void SetPhysicalAxisPos(const char* axisName, input::InputDevice device, uint16_t axisId) {
        auto item = std::find_if(axisMappings.begin(), axisMappings.end(), [axisName](auto& elem) { return elem.nameHash == shared::Hash(axisName); });

        if (item->physicalAxes.has_value() && item != axisMappings.end()) {
            item->physicalAxes.value().push_back(PhysicalAxis{.device = device, .axisId = axisId});
        }
        else {
            axisMappings.push_back(AxisMapping{
                .nameHash = shared::Hash(axisName),
                .virtualAxis = VirtualAxis{.positiveInputId = 0, .negativeInputId = 0 },
                .physicalAxes = std::vector {{ PhysicalAxis{.device = device, .axisId = axisId }} } });
        }
    }

    void SetVPhysicalAxisNeg(const char* axisName, input::InputDevice device, uint16_t axisId) {
        auto item = std::find_if(axisMappings.begin(), axisMappings.end(), [axisName](auto& elem) { return elem.nameHash == shared::Hash(axisName); });

        if (item->physicalAxes.has_value() && item != axisMappings.end()) {
            item->physicalAxes.value().push_back(PhysicalAxis{.device = device, .axisId = axisId});
        }
        else {
            axisMappings.push_back(AxisMapping{
                .nameHash = shared::Hash(axisName),
                .virtualAxis = VirtualAxis{.positiveInputId = 0, .negativeInputId = 0 },
                .physicalAxes = std::vector { {PhysicalAxis{.device = device, .axisId = axisId }} } });
        }
    }

    void SetButtonMapping(const char* buttonAction, input::InputDevice device, uint16_t buttonId) {
        auto item = std::find_if(buttonMappings.begin(), buttonMappings.end(), [buttonAction](auto& elem) { return elem.nameHash == shared::Hash(buttonAction); });

        if (item != buttonMappings.end()) {
            item->physicalButtons.push_back(PhysicalButton{ .device = device, .rawInputId = buttonId });
        }
        else {
            buttonMappings.push_back(ButtonMapping { .nameHash = shared::Hash(buttonAction), .physicalButtons = {PhysicalButton{.device = device, .rawInputId = buttonId }}});
        }
    }

    bool LoadInputMappings(const std::string& path,
        std::vector<AxisMapping>& axes,
        std::vector<ButtonMapping>& buttons) {
        mINI::INIFile file(path);
        mINI::INIStructure ini;
        if (!file.read(ini)) return false;

         for (const auto& section : ini) {
            const std::string& name = section.first;
            const auto& values = section.second;

            if (name.starts_with("axis:")) {
                AxisMapping mapping{};
                std::string label = name.substr(5);
                mapping.nameHash = shared::Hash(label);

                auto pos = values.get("virtual_pos");
                auto neg = values.get("virtual_neg");

                if (!pos.empty() && !neg.empty()) {
                    mapping.virtualAxis = VirtualAxis{
                        .positiveInputId = (uint16_t)std::stoi(pos),
                        .negativeInputId = (uint16_t)std::stoi(neg)
                    };
                }

                bool hasPhysicalDevice = false;
                bool hasPhysicalAxis = false;

                PhysicalAxis physical{};

                for (const auto& [key, val] : values) {
                    if (key == "p_device") {
                        physical.device = (input::InputDevice)std::stoi(val);
                        hasPhysicalDevice = true;
                    }

                    if (key == "p_axis") {
                        physical.axisId = std::stoi(val);
                        hasPhysicalAxis = true;
                    }
                }

                if (hasPhysicalDevice && hasPhysicalAxis) {
                    if (!mapping.physicalAxes.has_value()) {
                        mapping.physicalAxes = std::vector<PhysicalAxis> { physical };
                    }
                    else {
                        mapping.physicalAxes.value().push_back(physical);
                    }
                }

                axes.push_back(mapping);
            }
            else if (name.starts_with("button:")) {
                ButtonMapping mapping{};
                std::string label = name.substr(7);
                mapping.nameHash = shared::Hash(label);

                for (const auto& [key, val] : values) {
                    if (key == "physical") {
                        PhysicalButton b{};
                        b.device = input::InputDevice::Keyboard; // Default
                        b.rawInputId = std::stoi(val);
                        mapping.physicalButtons.push_back(b);
                    }
                }

                buttons.push_back(mapping);
            }
        }

        return true;
    }

    bool WriteInputMappings(const std::string& path,
        const std::vector<AxisMapping>& axes,
        const std::vector<ButtonMapping>& buttons) {
        mINI::INIStructure ini;

        for (const auto& a : axes) {
            std::string section = "axis:" + std::to_string(a.nameHash);
            if (a.virtualAxis.has_value()) {
                ini[section]["virtual_pos"] = std::to_string(a.virtualAxis.value().positiveInputId);
                ini[section]["virtual_neg"] = std::to_string(a.virtualAxis.value().negativeInputId);
            }

            if (!a.physicalAxes.has_value()) { continue; }
            for (const auto& p : a.physicalAxes.value()) {
                ini[section]["physical"] = std::to_string(p.axisId);
            }
        }

        for (const auto& b : buttons) {
            std::string section = "button:" + std::to_string(b.nameHash);
            for (const auto& p : b.physicalButtons) {
                ini[section]["physical"] = std::to_string(p.rawInputId); // Same here
            }
        }

        mINI::INIFile file(path);
        return file.write(ini);
    }
}


