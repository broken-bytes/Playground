#pragma once

#include "Mesh.hxx"
#include "Material.hxx"

#include <glm/glm.hpp>

namespace playground::rendering {
	class IMeshRenderer {
	public:
		virtual ~IMeshRenderer() = default;
		virtual void RenderMesh(Mesh& mesh, Material& material, glm::vec4 pos, glm::vec4 rotation, glm::vec3 scale) = 0;
	};
}