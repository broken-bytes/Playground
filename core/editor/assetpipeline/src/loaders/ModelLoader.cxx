#include "assetpipeline/loaders/ModelLoader.hxx"
#include <filesystem>
#include <stdexcept>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <rendering/Mesh.hxx>

namespace playground::editor::assetpipeline::loaders::modelloader {
	auto LoadFromFile(
		std::filesystem::path path
	) -> std::vector<assetloader::RawMeshData> {
		std::vector<assetloader::RawMeshData> meshes = {};

		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path.string(), 0);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
			auto err = importer.GetErrorString();
			throw std::runtime_error(err);
		}

		std::vector<assetloader::RawVertex> vertices;
		std::vector<uint32_t> indices = {};

		for (int x = 0; x < scene->mNumMeshes; x++) {
			// Process mesh
			aiMesh* mesh = scene->mMeshes[x];

			// Fill vertices
			for (int i = 0; i < mesh->mNumVertices; i++) {
				assetloader::RawVertex vertex = {};
				vertex.x = mesh->mVertices[i].x;
				vertex.y = mesh->mVertices[i].y;
				vertex.z = mesh->mVertices[i].z;

				vertex.nx = mesh->mNormals[i].x;
				vertex.ny = mesh->mNormals[i].y;
				vertex.nz = mesh->mNormals[i].z;

                if (mesh->mColors[0] != nullptr) {
                    vertex.cb = mesh->mColors[0][i].r;
                    vertex.cg = mesh->mColors[0][i].g;
                    vertex.cr = mesh->mColors[0][i].b;
                    vertex.ca = mesh->mColors[0][i].a;
                }
                else {
                    vertex.cb = 1;
                    vertex.cg = 1;
                    vertex.cr = 1;
                    vertex.ca = 1;
                }

				if (mesh->mTextureCoords[0]) {
					vertex.u = mesh->mTextureCoords[0][i].x;
					vertex.v = mesh->mTextureCoords[0][i].y;
				}
				else {
					vertex.u = 0;
					vertex.v = 0;
				}
				vertices.push_back(vertex);
			}

			// Fill indices
			for (int i = 0; i < mesh->mNumFaces; i++) {
				aiFace face = mesh->mFaces[i];
				for (int j = 0; j < face.mNumIndices; j++) {
					indices.push_back(face.mIndices[j]);
				}
			}

			// Create mesh
			assetloader::RawMeshData meshData;
			meshData.vertices = vertices;
			meshData.indices = indices;
			meshes.push_back(meshData);

			// Clear vectors
			vertices.clear();
			indices.clear();
		}

		return meshes;
	}

    auto LoadAnimationsFromFile(
        std::filesystem::path path
    ) -> std::vector<assetloader::RawAnimationData> {
        std::vector<assetloader::RawAnimationData> animations = {};

        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path.string(), 0);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            auto err = importer.GetErrorString();
            throw std::runtime_error(err);
        }
        for (int x = 0; x < scene->mNumAnimations; x++) {
            aiAnimation* animation = scene->mAnimations[x];

            assetloader::RawAnimationData rawAnimation = {};
            rawAnimation.name = animation->mName.C_Str();  
            rawAnimation.duration = animation->mDuration;  
            rawAnimation.framesPerSecond = animation->mTicksPerSecond;

            for (int i = 0; i < animation->mNumChannels; i++) {
                aiNodeAnim* channel = animation->mChannels[i];

                assetloader::RawAnimationChannel rawChannel = {};
                rawChannel.nodeName = channel->mNodeName.C_Str();

                // Process position keyframes
                for (int j = 0; j < channel->mNumPositionKeys; j++) {
                    aiVectorKey key = channel->mPositionKeys[j];
                    assetloader::RawKeyframe keyframe = {};
                    keyframe.time = key.mTime;
                    keyframe.px = key.mValue.x;
                    keyframe.py = key.mValue.y;
                    keyframe.pz = key.mValue.z;
                    rawChannel.positionKeys.push_back(keyframe);
                }

                // Process rotation keyframes
                for (int j = 0; j < channel->mNumRotationKeys; j++) {
                    aiQuatKey key = channel->mRotationKeys[j];
                    assetloader::RawKeyframe keyframe = {};
                    keyframe.time = key.mTime;
                    keyframe.rx = key.mValue.x;
                    keyframe.ry = key.mValue.y;
                    keyframe.rz = key.mValue.z;
                    keyframe.rw = key.mValue.w;

                    rawChannel.rotationKeys.push_back(keyframe);
                }

                // Process scaling keyframes
                for (int j = 0; j < channel->mNumScalingKeys; j++) {
                    aiVectorKey key = channel->mScalingKeys[j];
                    assetloader::RawKeyframe keyframe = {};
                    keyframe.time = key.mTime;
                    keyframe.sx = key.mValue.x;
                    keyframe.sy = key.mValue.y;
                    keyframe.sz = key.mValue.z;

                    rawChannel.scalingKeys.push_back(keyframe);
                }

                rawAnimation.channels.push_back(rawChannel);
            }

            animations.push_back(rawAnimation);
        }

        return animations;
    }
}
