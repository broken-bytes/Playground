#pragma once

#include "GpuResource.hxx"

namespace playground::rendering {
	class RenderTarget : public GpuResource {
	public:
        explicit RenderTarget() : GpuResource() {}

        virtual ~RenderTarget() = default;
	};
}
