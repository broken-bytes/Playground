#pragma once

#include <string>
#include <vector>


namespace playground::rendering {
    enum class ShaderType {
        Vertex,
        Pixel,
        Compute
    };
    struct Shader {
    public:
        Shader(uint32_t handle) : _handle(handle) {}
        virtual ~Shader() = default;

    private:
        uint32_t _handle;
    };
}
