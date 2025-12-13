#pragma once

#include <cstdint>

namespace playground::rendering {
    class CPUResourceHandle {
    public:
        CPUResourceHandle(uint32_t id) : _id(id) {}

        virtual ~CPUResourceHandle() = default;

        auto ID() const -> uint32_t {
            return _id;
        }

    private:
        uint32_t _id;
    };
}
