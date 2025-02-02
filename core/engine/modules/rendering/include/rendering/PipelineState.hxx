#pragma once

#include "PrimitiveTopology.hxx"
#include "Shader.hxx"

#include <memory>

namespace playground::rendering {
	class PipelineState {
	public:

		virtual ~PipelineState() = default;
	};
}