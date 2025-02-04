#pragma once

namespace playground::rendering {
    enum class TextureFiltering {
        Point,
        Bilinear,
        Trilinear,
        Anisotropic
    };

    enum class TextureWrapping {
        Repeat,
        Clamp,
        Mirror
    };

    class Sampler {
    public:
        virtual ~Sampler() = default;
    };
}
