#pragma once

namespace playground::rendering {
    class Texture {
    public:
        virtual ~Texture() = default;

        virtual uint32_t ID() const = 0;
    };
}

