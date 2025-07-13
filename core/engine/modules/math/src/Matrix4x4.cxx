#include "math/Matrix4x4.hxx"
#include "math/Matrix3x3.hxx"
#include "math/Vector4.hxx"

namespace playground::math {
    Matrix4x4::Matrix4x4() : elements{
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    } {
    }

    Matrix4x4 Matrix4x4::Zero() {
        auto mat = Matrix4x4();
        for (int x = 0; x < 4; x++) {
            for (int y = 0; y < 4; y++) {
                mat.elements[x][y] = 0.0f;
            }
        }

        return mat;
    }

    Matrix4x4 Matrix4x4::operator*(const Matrix4x4& other) const {
        Matrix4x4 result;
        for (int row = 0; row < 4; ++row) {
            for (int col = 0; col < 4; ++col) {
                for (int k = 0; k < 4; ++k) {
                    result.elements[row][col] += elements[row][k] * other.elements[k][col];
                }
            }
        }

        return result;
    }

    Vector4 Matrix4x4::operator*(const Vector4& v) const {
        return Vector4(
            elements[0][0] * v.X + elements[0][1] * v.Y + elements[0][2] * v.Z + elements[0][3] * v.W,
            elements[1][0] * v.X + elements[1][1] * v.Y + elements[1][2] * v.Z + elements[1][3] * v.W,
            elements[2][0] * v.X + elements[2][1] * v.Y + elements[2][2] * v.Z + elements[2][3] * v.W,
            elements[3][0] * v.X + elements[3][1] * v.Y + elements[3][2] * v.Z + elements[3][3] * v.W
        );
    }

    Matrix4x4& Matrix4x4::operator*=(const Matrix4x4& other) {
        *this = *this * other;
        return *this;
    }

    Matrix4x4& Matrix4x4::operator+=(const Matrix4x4& other) {
        for (int row = 0; row < 4; ++row) {
            for (int col = 0; col < 4; ++col) {
                elements[row][col] += other.elements[row][col];
            }
        }
        return *this;
    }

    Matrix4x4 Matrix4x4::operator-(const Matrix4x4& other) const {
        Matrix4x4 result;
        for (int row = 0; row < 4; ++row) {
            for (int col = 0; col < 4; ++col) {
                result.elements[row][col] = elements[row][col] - other.elements[row][col];
            }
        }

        return result;
    }

    Matrix4x4& Matrix4x4::operator-=(const Matrix4x4& other) {
        for (int row = 0; row < 4; ++row) {
            for (int col = 0; col < 4; ++col) {
                elements[row][col] -= other.elements[row][col];
            }
        }

        return *this;
    }

    bool Matrix4x4::operator==(const Matrix4x4& other) const {
        for (int row = 0; row < 4; ++row) {
            for (int col = 0; col < 4; ++col) {
                if (elements[row][col] != other.elements[row][col])
                    return false;
            }
        }

        return true;
    }

    bool Matrix4x4::operator!=(const Matrix4x4& other) const {
        return !(*this == other);
    }

    float& Matrix4x4::operator()(int row, int col) {
        return elements[row][col];
    }

    const float& Matrix4x4::operator()(int row, int col) const {
        return elements[row][col];
    }

    const float* Matrix4x4::Row(int row) const {
        return elements[row].data();
    }

    static Matrix4x4 Identity() {
        Matrix4x4 result{};

        for (int i = 0; i < 4; ++i) {
            result.elements[i][i] = 1.0f;
        }

        return result;
    }

    std::string Matrix4x4::ToString() const {
        std::string result = "Matrix4x4:\n";
        for (int row = 0; row < 4; ++row) {
            result += "| ";
            for (int col = 0; col < 4; ++col) {
                result += std::to_string(elements[row][col]) + " ";
            }
            result += "|\n";
        }

        return result;
    }

    Matrix3x3 Matrix4x4::ToMatrix3x3() const {
        auto matrix3x3 = Matrix3x3();
        for (int row = 0; row < 3; ++row) {
            for (int col = 0; col < 3; ++col) {
                matrix3x3(row, col) = elements[row][col];
            }
        }

        return matrix3x3;
    }
}
