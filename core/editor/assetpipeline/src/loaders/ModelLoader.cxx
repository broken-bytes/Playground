#include "assetpipeline/loaders/ModelLoader.hxx"
#include <filesystem>
#include <stdexcept>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <glm/glm.hpp>
#include <rendering/Mesh.hxx>

namespace playground::editor::assetpipeline::loaders::modelloader {
    void ProcessNode(aiNode* node, const aiScene* scene, const aiMatrix4x4& parentTransform, std::vector<assetloader::RawMeshData>& outMeshes, std::unordered_set<uint32_t>& processedMeshes) {
        aiMatrix4x4 localTransform = node->mTransformation;
        aiMatrix4x4 worldTransform = parentTransform * localTransform;

        for (auto& item : processedMeshes) {
            std::cout << "Processed mesh ID: " << item << std::endl;
        }

        for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            if (processedMeshes.find(node->mMeshes[i]) != processedMeshes.end()) {
                std::cout << "Skipping already processed mesh: " << mesh->mName.C_Str() << std::endl;
                continue; // Skip already processed meshes
            }

            std::cout << "Processing node: " << node->mName.C_Str() << ", mesh: " << mesh->mName.C_Str() << " ID: " << +node->mMeshes[i] << std::endl;

            processedMeshes.insert(node->mMeshes[i]);

            std::vector<assetloader::RawVertex> vertices;
            std::vector<uint32_t> indices = {};

            std::cout << "Processing mesh: " << mesh->mName.C_Str() << std::endl;

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
                    vertex.u = mesh->mTextureCoords[0][i].y;
                    vertex.v = 1 - mesh->mTextureCoords[0][i].x;
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

            // Decompose world transform
            aiVector3D scale, position;
            aiQuaternion rotation;
            worldTransform.Decompose(scale, rotation, position);

            assetloader::RawMeshData meshData;
            meshData.posX = position.x;
            meshData.posY = position.y;
            meshData.posZ = position.z;
            meshData.rotX = rotation.x;
            meshData.rotY = rotation.y;
            meshData.rotZ = rotation.z;
            meshData.rotW = rotation.w;
            meshData.scaleX = scale.x;
            meshData.scaleY = scale.y;
            meshData.scaleZ = scale.z;
            std::string name;
            name.append(node->mName.C_Str());
            name.append("_");
            name.append(mesh->mName.C_Str());
            meshData.name = name;
            meshData.vertices = vertices;
            meshData.indices = indices;
            outMeshes.push_back(meshData);

            // Clear vectors
            vertices.clear();
            indices.clear();
        }

        for (unsigned int i = 0; i < node->mNumChildren; ++i) {
            ProcessNode(node->mChildren[i], scene, worldTransform, outMeshes, processedMeshes);
        }
    }

    auto LoadFromFile(
        std::filesystem::path path
    ) -> std::vector<assetloader::RawMeshData> {
        std::vector<assetloader::RawMeshData> meshes = {};
        std::unordered_set<uint32_t> processedMeshes;

        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path.string(), 0);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            auto err = importer.GetErrorString();
            throw std::runtime_error(err);
        }

        ProcessNode(scene->mRootNode, scene, aiMatrix4x4(), meshes, processedMeshes);

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
