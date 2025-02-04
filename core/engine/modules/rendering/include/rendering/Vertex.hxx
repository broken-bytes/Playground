#pragma once

#include <glm/glm.hpp>

namespace playground::rendering {
	struct Vertex {
		glm::vec3 position;
        glm::vec4 color;
		glm::vec3 normal;
		glm::vec2 uvs;
	};
}
