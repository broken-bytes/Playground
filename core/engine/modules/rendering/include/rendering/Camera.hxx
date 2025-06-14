#pragma once

#undef near
#undef far
#include <cstdint>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace playground::rendering {
    struct CameraData {
        glm::mat4x4 ViewMatrix;
        glm::mat4x4 ProjectionMatrix;
    };

    struct Camera {
        glm::vec3 Position;
        glm::quat Rotation;
        float FOV;
        float AspectRatio;
        float Near;
        float Far;
        uint32_t RenderTargetTextureId;

        Camera() {
            FOV = 45.0f;
            AspectRatio = 1.0f;
            Near = 0.1f;
            Far = 100.0f;
            Position = glm::vec3(0.0f, 0.0f, 0.0f);
            Rotation = glm::quat(0.0f, 0.0f, 0.0f, 1);
            RenderTargetTextureId = 0;
        }

        Camera(
            float fov,
            float aspectRatio,
            float near,
            float far,
            glm::vec3 pos,
            glm::quat rot,
            uint32_t renderTargetTextureId
        ) : FOV(fov), AspectRatio(aspectRatio), Near(near), Far(far), Position(pos), Rotation(rot), RenderTargetTextureId(renderTargetTextureId) {}

        float GetFov() const { return FOV; }
        float GetAspectRatio() const { return AspectRatio; }
        float GetNear() const { return Near; }
        float GetFar() const { return Far; }
        glm::vec3 GetPosition() const { return Position; }
        glm::quat GetRotation() const { return Rotation; }

        void SetFov(float fov) { FOV = fov; }
        void SetAspectRatio(float aspectRatio) { AspectRatio = aspectRatio; }
        void SetNear(float nearPlane) { Near = nearPlane; }
        void SetFar(float farPlane) { Far = farPlane; }
        void SetPosition(glm::vec3 pos) { Position = pos; }
        void SetRotation(glm::vec3 rot) { Rotation = rot; }

        glm::mat4 GetViewMatrix() const {
            // Construct the rotation matrix manually
            glm::mat4 rotationMatrix = glm::mat4(
                1 - 2 * (Rotation.y * Rotation.y + Rotation.z * Rotation.z),
                2 * (Rotation.x * Rotation.y - Rotation.w * Rotation.z),
                2 * (Rotation.x * Rotation.z + Rotation.w * Rotation.y),
                0,
                2 * (Rotation.x * Rotation.y + Rotation.w * Rotation.z),
                1 - 2 * (Rotation.x * Rotation.x + Rotation.z * Rotation.z),
                2 * (Rotation.y * Rotation.z - Rotation.w * Rotation.x),
                0,
                2 * (Rotation.x * Rotation.z - Rotation.w * Rotation.y),
                2 * (Rotation.y * Rotation.z + Rotation.w * Rotation.x),
                1 - 2 * (Rotation.x * Rotation.x + Rotation.y * Rotation.y),
                0,
                0,
                0,
                0,
                1
            );

            glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), -Position);

            return rotationMatrix * translationMatrix;
        }

        glm::mat4 GetProjectionMatrix() const {
            return glm::perspectiveLH(glm::radians(FOV), AspectRatio, Near, Far);
        }
    };
}
