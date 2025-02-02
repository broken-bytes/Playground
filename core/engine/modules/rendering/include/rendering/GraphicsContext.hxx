#pragma once

#include <memory>
#include "rendering/Context.hxx"
#include "rendering/RenderTarget.hxx"

namespace playground::rendering {
    class GraphicsContext : public Context {
    public:
        virtual auto CopyToBackBuffer(std::shared_ptr<RenderTarget> renderTarget) -> void = 0;
    };
}
