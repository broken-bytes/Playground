#pragma once

#include "rendering/VertexBufferHandle.hxx"
#include "rendering/IndexBufferHandle.hxx"
#include "rendering/MaterialHandle.hxx"
#include "rendering/Constants.hxx"
#include <glm/glm.hpp>
#include <EASTL/vector.h>

namespace playground::rendering {
	struct DrawCall {
        struct InstanceData {
            glm::mat4 transform;
            glm::mat4 normals;
        };
        VertexBufferHandle vertexBuffer;
        IndexBufferHandle indexBuffer;
        MaterialHandle material;
        eastl::vector<InstanceData> instanceData;
	};
}
