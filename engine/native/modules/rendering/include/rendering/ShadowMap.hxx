#pragma once

#include "rendering/DepthBuffer.hxx"
#include <math/Matrix4x4.hxx>
#include <cstdint>

namespace playground::rendering {
    class ShadowMap {
    public:
        ShadowMap(uint16_t width, uint16_t height) : _width(width), _height(height) {}

        uint16_t Width() const {
            return _width;
        }

        uint16_t Height() const {
            return _height;
        }

        virtual std::shared_ptr<DepthBuffer> GetDepthBuffer() = 0;
        virtual uint32_t ID() const = 0;

    private:
        uint16_t _width;
        uint16_t _height;
    };
}
