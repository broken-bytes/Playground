#pragma once

#include "rendering/VertexBufferHandle.hxx"
#include "rendering/IndexBufferHandle.hxx"
#include "rendering/MaterialHandle.hxx"
#include "rendering/Constants.hxx"
#include <math/Matrix3x3.hxx>
#include <math/Matrix4x4.hxx>
#include <EASTL/vector.h>

namespace playground::rendering {
	struct DrawCall {
        struct InstanceData {
            math::Matrix4x4 transform;
            math::Matrix4x4 normals;
        };
        VertexBufferHandle vertexBuffer;
        IndexBufferHandle indexBuffer;
        MaterialHandle material;
        eastl::vector<InstanceData> instanceData;
	};
}
