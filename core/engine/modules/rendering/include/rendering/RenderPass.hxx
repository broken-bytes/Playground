#pragma once

#include <memory>
#include "rendering/RenderTarget.hxx"
#include "rendering/DepthBuffer.hxx"
#include "rendering/CommandList.hxx"

namespace playground::rendering {
	class RenderPass {
	public:
		virtual ~RenderPass() = default;
		virtual void Begin(std::shared_ptr<CommandList> list, std::shared_ptr<RenderTarget> colour, std::shared_ptr<DepthBuffer> depth) = 0;
		virtual void Execute() = 0;
		virtual void End() = 0;
	};
}
