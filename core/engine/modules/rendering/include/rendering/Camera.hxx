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

    class Camera {
    public:
        Camera() {
            _fov = 45.0f;
            _aspectRatio = 1.0f;
            _near = 0.1f;
            _far = 100.0f;
            _position = glm::vec3(0.0f, 0.0f, 0.0f);
            _rotation = glm::vec3(0.0f, 0.0f, 0.0f);
        }

        Camera(
            float fov,
            float aspectRatio,
            float near,
            float far,
            glm::vec3 pos,
            glm::vec3 rot
        ) : _fov(fov), _aspectRatio(aspectRatio), _near(near), _far(far), _position(pos), _rotation(rot) {}

        float GetFov() const { return _fov; }
        float GetAspectRatio() const { return _aspectRatio; }
        float GetNear() const { return _near; }
        float GetFar() const { return _far; }
        glm::vec3 GetPosition() const { return _position; }
        glm::vec3 GetRotation() const { return _rotation; }

        void SetFov(float fov) { _fov = fov; }
        void SetAspectRatio(float aspectRatio) { _aspectRatio = aspectRatio; }
        void SetNear(float nearPlane) { _near = nearPlane; }
        void SetFar(float farPlane) { _far = farPlane; }
        void SetPosition(glm::vec3 pos) { _position = pos; }
        void SetRotation(glm::vec3 rot) { _rotation = rot; }

        glm::mat4 GetViewMatrix() const {
            glm::quat q = glm::quat(_rotation); // Convert Euler angles to quaternion

            // Construct the rotation matrix manually
            glm::mat4 rotationMatrix = glm::mat4(
                1 - 2 * (q.y * q.y + q.z * q.z), 2 * (q.x * q.y - q.w * q.z), 2 * (q.x * q.z + q.w * q.y), 0,
                2 * (q.x * q.y + q.w * q.z), 1 - 2 * (q.x * q.x + q.z * q.z), 2 * (q.y * q.z - q.w * q.x), 0,
                2 * (q.x * q.z - q.w * q.y), 2 * (q.y * q.z + q.w * q.x), 1 - 2 * (q.x * q.x + q.y * q.y), 0,
                0, 0, 0, 1
            );

            glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), -_position);

            return rotationMatrix * translationMatrix;
        }

        glm::mat4 GetProjectionMatrix() const {
            return glm::perspectiveLH(glm::radians(_fov), _aspectRatio, _near, _far);
        }

        CameraData GetCameraData() const {
            CameraData data;
            data.ViewMatrix = glm::transpose(GetViewMatrix());
            data.ProjectionMatrix = glm::transpose(GetProjectionMatrix());
            return data;
        }

    private:
        glm::vec3 _position;
        glm::vec3 _rotation;
        float _fov;
        float _aspectRatio;
        float _near;
        float _far;
    };
}
