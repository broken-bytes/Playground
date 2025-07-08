#include "math/Matrix3x3.hxx"
#include "math/Matrix4x4.hxx"

namespace playground::math {
    Matrix3x3::Matrix3x3()
        : elements{ 1.0f, 0.0f, 0.0f,
                   0.0f, 1.0f, 0.0f,
                   0.0f, 0.0f, 1.0f } {
    }

    Matrix3x3::Matrix3x3(const std::array<float, 9>& elems)
        : elements(elems) {
    }

    float& Matrix3x3::operator()(int row, int col) {
        return elements[row * 3 + col];
    }

    const float& Matrix3x3::operator()(int row, int col) const {
        return elements[row * 3 + col];
    }

    const float* Matrix3x3::Row(int row) const {
        return elements.data() + row * 3;
    }

    Matrix3x3 Matrix3x3::operator*(const Matrix3x3& other) const {
        Matrix3x3 result;
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                result(i, j) = (*this)(i, 0) * other(0, j) +
                    (*this)(i, 1) * other(1, j) +
                    (*this)(i, 2) * other(2, j);
            }
        }
        return result;
    }

    bool Matrix3x3::operator==(const Matrix3x3& other) const {
        for (int i = 0; i < 9; ++i) {
            if (elements[i] != other.elements[i]) return false;
        }
        return true;
    }

    bool Matrix3x3::operator!=(const Matrix3x3& other) const {
        return !(*this == other);
    }

    Matrix4x4 Matrix3x3::ToMatrix4x4() const {
        return Matrix4x4({
            elements[0], elements[1], elements[2], 0.0f,
            elements[3], elements[4], elements[5], 0.0f,
            elements[6], elements[7], elements[8], 0.0f,
            0.0f,        0.0f,        0.0f,        1.0f
            });
    }

    std::string Matrix3x3::ToString() const {
        std::string result = "Matrix3x3:\n";
        for (int i = 0; i < 3; ++i) {
            result += "| ";
            for (int j = 0; j < 3; ++j) {
                result += std::to_string((*this)(i, j)) + " ";
            }
            result += "|\n";
        }
        return result;
    }
}
