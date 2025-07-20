#include "math/Quaternion.hxx"
#include "math/Vector3.hxx"

namespace playground::math {
    Quaternion Quaternion::Normalise() {
        float length = sqrt(X * X + Y * Y + Z * Z + W * W);
        if (length > 0.00001f) {
            float invLen = 1.0f / length;
            return Quaternion{ X * invLen, Y * invLen, Z * invLen, W * invLen };
        }
        else {
            // If length is too small, return identity quaternion as a fallback
            return Quaternion{ 0.0f, 0.0f, 0.0f, 1.0f };
        }
    }

    Vector3 Quaternion::Forward() const {
        return Vector3(
            2.0f * (X * Z + W * Y),
            2.0f * (Y * Z - W * X),
            1.0f - 2.0f * (X * X + Y * Y)
        );
    }

    Quaternion Quaternion::LookRotation(const Vector3& forward, const Vector3& up) {
        Vector3 f = forward.Normalise();
        Vector3 u = up.Normalise();
        Vector3 r = u.Cross(f).Normalise();      // Right vector
        u = f.Cross(r);                         // Recompute orthogonal up

        // Construct rotation matrix columns (row-major)
        // [ r.X u.X f.X ]
        // [ r.y u.y f.y ]
        // [ r.Z u.Z f.Z ]

        float trace = r.X + u.Y + f.Z;
        Quaternion q;

        if (trace > 0) {
            float s = sqrt(trace + 1.0f) * 2; // s=4*qw
            q.W = 0.25f * s;
            q.X = (u.Z - f.Y) / s;
            q.Y = (f.X - r.Z) / s;
            q.Z = (r.Y - u.X) / s;
        }
        else if ((r.X > u.Y) && (r.X > f.Z)) {
            float s = sqrt(1.0f + r.X - u.Y - f.Z) * 2; // s=4*qx
            q.W = (u.Z - f.Y) / s;
            q.X = 0.25f * s;
            q.Y = (u.X + r.Y) / s;
            q.Z = (f.X + r.Z) / s;
        }
        else if (u.Y > f.Z) {
            float s = sqrt(1.0f + u.Y - r.X - f.Z) * 2; // s=4*qy
            q.W = (f.X - r.Z) / s;
            q.X = (u.X + r.Y) / s;
            q.Y = 0.25f * s;
            q.Z = (f.Y + u.Z) / s;
        }
        else {
            float s = sqrt(1.0f + f.Z - r.X - u.Y) * 2; // s=4*qz
            q.W = (r.Y - u.X) / s;
            q.X = (f.X + r.Z) / s;
            q.Y = (f.Y + u.Z) / s;
            q.Z = 0.25f * s;
        }

        return q.Normalise();
    }
}
