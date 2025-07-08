#pragma once

#undef near
#undef far
#include <cstdint>
#include <math/Math.hxx>
#include <math/Vector3.hxx>
#include <math/Quaternion.hxx>
#include <math/Matrix4x4.hxx>

namespace playground::rendering {
    struct Camera {
        math::Vector3 Position;
        math::Quaternion Rotation;
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
            Position = math::Vector3(0.0f, 0.0f, 0.0f);
            Rotation = math::Quaternion(0.0f, 0.0f, 0.0f, 1);
            RenderTargetTextureId = 0;
        }

        Camera(
            float fov,
            float aspectRatio,
            float near,
            float far,
            math::Vector3 pos,
            math::Quaternion rot,
            uint32_t renderTargetTextureId
        ) : FOV(fov), AspectRatio(aspectRatio), Near(near), Far(far), Position(pos), Rotation(rot), RenderTargetTextureId(renderTargetTextureId) {}

        float GetFov() const { return FOV; }
        float GetAspectRatio() const { return AspectRatio; }
        float GetNear() const { return Near; }
        float GetFar() const { return Far; }
        math::Vector3 GetPosition() const { return Position; }
        math::Quaternion GetRotation() const { return Rotation; }

        void SetFov(float fov) { FOV = fov; }
        void SetAspectRatio(float aspectRatio) { AspectRatio = aspectRatio; }
        void SetNear(float nearPlane) { Near = nearPlane; }
        void SetFar(float farPlane) { Far = farPlane; }
        void SetPosition(math::Vector3 pos) { Position = pos; }
        void SetRotation(math::Quaternion rot) { Rotation = rot; }

        math::Matrix4x4 GetViewMatrix() const {
            math::Quaternion qInv = Rotation.Inverse();       // Inverse rotation (world -> view)
            math::Matrix4x4 rot = qInv.ToMatrix();             // 3x3 rotation matrix
            math::Vector3 t = -(qInv * Position);              // Transformed -camera position

            math::Matrix4x4 view;

            // Set rotation part (upper-left 3x3)
            view(0, 0) = rot(0, 0); view(0, 1) = rot(0, 1); view(0, 2) = rot(0, 2); view(0, 3) = 0.0f;
            view(1, 0) = rot(1, 0); view(1, 1) = rot(1, 1); view(1, 2) = rot(1, 2); view(1, 3) = 0.0f;
            view(2, 0) = rot(2, 0); view(2, 1) = rot(2, 1); view(2, 2) = rot(2, 2); view(2, 3) = 0.0f;

            // Translation in the last row (row-major layout)
            view(3, 0) = t.X;
            view(3, 1) = t.Y;
            view(3, 2) = t.Z;
            view(3, 3) = 1.0f;

            return view;
        }

        math::Matrix4x4 GetProjectionMatrix() const {
            math::Matrix4x4 projectionMatrix;
            math::GetProjectionMatrix(FOV, AspectRatio, Near, Far, &projectionMatrix);

            return projectionMatrix;
        }
    };
}
