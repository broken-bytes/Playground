#pragma once

#include "Mesh.hxx"

#include <cereal/access.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>

#include <vector>

namespace playground::rendering {
	struct ModelData {
		std::vector<MeshData> meshes;

		ModelData() = default;

		ModelData(std::vector<MeshData>& meshes) {
			std::move(meshes.begin(), meshes.end(), std::back_inserter(this->meshes));
		}

	private:
		friend class cereal::access;

		template <class Archive>
		void serialize(Archive& ar) {
			ar(meshes);
		}
	};

	struct Model {

	};
}