#include "math/Vector4.hxx"
#include "math/Matrix4x4.hxx"

namespace playground::math {
    Vector4 Vector4::operator*(const Matrix4x4& m) const {
        return Vector4(
            X * m.elements[0][0] + Y * m.elements[1][0] + Z * m.elements[2][0] + W * m.elements[3][0],
            X * m.elements[0][1] + Y * m.elements[1][1] + Z * m.elements[2][1] + W * m.elements[3][1],
            X * m.elements[0][2] + Y * m.elements[1][2] + Z * m.elements[2][2] + W * m.elements[3][2],
            X * m.elements[0][3] + Y * m.elements[1][3] + Z * m.elements[2][3] + W * m.elements[3][3]
        );
    }
}
