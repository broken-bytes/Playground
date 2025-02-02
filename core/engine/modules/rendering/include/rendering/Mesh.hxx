#pragma once

#include "rendering/Vertex.hxx"

#include <cstdint>
#include <string>
#include <vector>

namespace playground::rendering {
	struct MeshData {
		std::string name;
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
	};

	struct Mesh {
		uint32_t vertexBuffer;
		uint32_t indexBuffer;
		uint32_t vertexCount;
		uint32_t indexCount;
	};
}