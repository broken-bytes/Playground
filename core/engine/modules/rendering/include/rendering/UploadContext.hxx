#pragma once

#include <memory>
#include "rendering/Context.hxx"
#include "rendering/IndexBuffer.hxx"
#include "rendering/VertexBuffer.hxx"
#include "rendering/InstanceBuffer.hxx"
#include "rendering/Texture.hxx"

namespace playground::rendering {
    class UploadContext : public Context {
    public:
        virtual ~UploadContext() = default;

        virtual auto Upload(std::shared_ptr<Texture> texture) -> void = 0;
        virtual auto Upload(std::shared_ptr<IndexBuffer> buffer) -> void = 0;
        virtual auto Upload(std::shared_ptr<VertexBuffer> buffer) -> void = 0;
        virtual auto Upload(std::shared_ptr<InstanceBuffer> buffer) -> void = 0;
    };
}
