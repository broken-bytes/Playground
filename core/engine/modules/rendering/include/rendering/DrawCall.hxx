#pragma once

#include "rendering/VertexBufferHandle.hxx"
#include "rendering/IndexBufferHandle.hxx"
#include "rendering/MaterialHandle.hxx"
#include <glm/glm.hpp>
#include <vector>

namespace playground::rendering {
	struct DrawCall {
        struct InstanceData {
            glm::mat4 transform;
        };
        uint16_t instances;
        VertexBufferHandle vertexBuffer;
        IndexBufferHandle indexBuffer;
        MaterialHandle material;
        std::vector<InstanceData> instanceData;
	};
}
