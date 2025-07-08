#include "math/Matrix4x4.hxx"
#include "math/Matrix3x3.hxx"

namespace playground::math {
    Matrix4x4::Matrix4x4() : elements{ 1.0f, 0.0f, 0.0f, 0.0f,
                        0.0f, 1.0f, 0.0f, 0.0f,
                        0.0f, 0.0f, 1.0f, 0.0f,
                        0.0f, 0.0f, 0.0f, 1.0f } {
    }

    Matrix4x4::Matrix4x4(const std::array<float, 16>& elements) : elements(elements) {}

    Matrix4x4 Matrix4x4::operator*(const Matrix4x4& other) const {
        Matrix4x4 result;
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                result.elements[i * 4 + j] = elements[i * 4 + 0] * other.elements[0 * 4 + j] +
                    elements[i * 4 + 1] * other.elements[1 * 4 + j] +
                    elements[i * 4 + 2] * other.elements[2 * 4 + j] +
                    elements[i * 4 + 3] * other.elements[3 * 4 + j];
            }
        }
        return result;
    }

    Matrix4x4& Matrix4x4::operator*=(const Matrix4x4& other) {
        *this = *this * other;
        return *this;
    }

    Matrix4x4 Matrix4x4::operator+(const Matrix4x4& other) const {
        Matrix4x4 result;
        for (int i = 0; i < 16; ++i) {
            result.elements[i] = elements[i] + other.elements[i];
        }
        return result;
    }

    Matrix4x4& Matrix4x4::operator+=(const Matrix4x4& other) {
        for (int i = 0; i < 16; ++i) {
            elements[i] += other.elements[i];
        }
        return *this;
    }

    Matrix4x4 Matrix4x4::operator-(const Matrix4x4& other) const {
        Matrix4x4 result;
        for (int i = 0; i < 16; ++i) {
            result.elements[i] = elements[i] - other.elements[i];
        }
        return result;
    }

    Matrix4x4& Matrix4x4::operator-=(const Matrix4x4& other) {
        for (int i = 0; i < 16; ++i) {
            elements[i] -= other.elements[i];
        }
        return *this;
    }

    bool Matrix4x4::operator==(const Matrix4x4& other) const {
        for (int i = 0; i < 16; ++i) {
            if (elements[i] != other.elements[i]) {
                return false;
            }
        }
        return true;
    }

    bool Matrix4x4::operator!=(const Matrix4x4& other) const {
        return !(*this == other);
    }

    float& Matrix4x4::operator()(int row, int col) {
        return elements[row * 4 + col];
    }

    const float& Matrix4x4::operator()(int row, int col) const {
        return elements[row * 4 + col];
    }

    const float* Matrix4x4::Row(int row) const {
        return elements.data() + row * 4;
    }

    static Matrix4x4 Identity() {
        return Matrix4x4();
    }

    std::string Matrix4x4::ToString() const {
        std::string result = "Matrix4x4:\n";
        for (int i = 0; i < 4; ++i) {
            result += "| ";
            for (int j = 0; j < 4; ++j) {
                result += std::to_string(elements[i * 4 + j]) + " ";
            }
            result += "|\n";
        }
        return result;
    }

    Matrix3x3 Matrix4x4::ToMatrix3x3() const {
        return Matrix3x3({
            elements[0], elements[1], elements[2],
            elements[4], elements[5], elements[6],
            elements[8], elements[9], elements[10]
        });
    }
}
