#pragma once

#include <math/Vector2.hxx>
#include <math/Vector3.hxx>
#include <math/Vector4.hxx>

namespace playground::rendering {
	struct Vertex {
		math::Vector3 position;
        math::Vector4 color;
        math::Vector3 normal;
        math::Vector2 uvs;
	};
}
