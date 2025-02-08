#pragma once

#include <memory>
#include "rendering/Context.hxx"
#include "rendering/RenderTarget.hxx"
#include "rendering/IndexBuffer.hxx"
#include "rendering/VertexBuffer.hxx"
#include "rendering/Texture.hxx"

namespace playground::rendering {
    class GraphicsContext : public Context {
    public:
        virtual auto TransitionIndexBuffer(std::shared_ptr<IndexBuffer> buffer) -> void = 0;
        virtual auto TransitionVertexBuffer(std::shared_ptr<VertexBuffer> buffer) -> void = 0;
        virtual auto TransitionTexture(std::shared_ptr<Texture> texture) -> void = 0;
        virtual auto CopyToBackBuffer(std::shared_ptr<RenderTarget> renderTarget) -> void = 0;
        virtual auto ReadbackBuffer(void* data, size_t* numBytes) -> void = 0;
        virtual auto ExecuteCommandLists(std::vector<std::shared_ptr<CommandList>> commandLists) -> void = 0;
        virtual auto MouseOverID() -> uint64_t = 0;
    };
}
