#pragma once

#include <glm/glm.hpp>
#include "rendering/CommandListType.hxx"
#include "rendering/Vertex.hxx"
#include "rendering/IndexBuffer.hxx"
#include "rendering/PrimitiveTopology.hxx"
#include "rendering/VertexBuffer.hxx"
#include "rendering/Mesh.hxx"
#include "rendering/Material.hxx"
#include "rendering/CPUResourceHandle.hxx"
#include "rendering/RenderTarget.hxx"
#include "rendering/DepthBuffer.hxx"
#include "rendering/ConstantBuffer.hxx"
#include "rendering/Texture.hxx"
#include "rendering/Sampler.hxx"
#include "rendering/ReadbackBuffer.hxx"

namespace playground::rendering {
	class CommandList {
	public:
		explicit CommandList(CommandListType type) : _type(type) {};
		virtual ~CommandList() = default;
		virtual auto Begin() -> void = 0;
		virtual auto Close() -> void = 0;
        virtual auto Reset() -> void = 0;
        virtual auto SetRenderTarget(std::shared_ptr<RenderTarget> colour, std::shared_ptr<DepthBuffer> depth) -> void = 0;
        virtual auto ClearDepthTarget(std::shared_ptr<DepthBuffer> target, float depth) -> void = 0;
		virtual auto ClearRenderTarget(std::shared_ptr<RenderTarget> handle, glm::vec4 color) -> void = 0;
		virtual auto SetViewport(
			uint32_t x,
			uint32_t y,
			uint32_t width,
			uint32_t height,
			uint32_t minDepth,
			uint32_t maxDepth
		) -> void = 0;
		virtual auto SetScissorRect(uint32_t left, uint32_t top, uint32_t right, uint32_t bottom) -> void = 0;
		virtual auto SetPrimitiveTopology(PrimitiveTopology topology) -> void = 0;
		virtual auto SetMaterial(std::shared_ptr<Material>& material) -> void = 0;
		virtual auto BindVertexBuffer(std::shared_ptr<VertexBuffer>& vertexBuffer, uint8_t slot) -> void = 0;
		virtual auto BindIndexBuffer(std::shared_ptr<IndexBuffer>& vertexBuffer) -> void = 0;
        virtual auto BindConstantBuffer(std::shared_ptr<ConstantBuffer> buffer, uint8_t slot) -> void = 0;
        virtual auto BindTexture(std::shared_ptr<Texture> texture, uint8_t slot) -> void = 0;
        virtual auto BindSampler(std::shared_ptr<Sampler> sampler, uint8_t slot) -> void = 0;
		virtual auto DrawIndexed(uint32_t numIndices, uint32_t startIndex, uint32_t startVertex) -> void = 0;

		auto Type() const -> CommandListType { return _type; }

	protected:
		CommandListType _type;
	};
}
