#include "math/Matrix3x3.hxx"
#include "math/Matrix4x4.hxx"

namespace playground::math {
    Matrix3x3::Matrix3x3()
        : elements{ 1.0f, 0.0f, 0.0f,
                   0.0f, 1.0f, 0.0f,
                   0.0f, 0.0f, 1.0f } {
    }


    float& Matrix3x3::operator()(int row, int col) {
        return elements[row][col];
    }

    const float& Matrix3x3::operator()(int row, int col) const {
        return elements[row][col];
    }

    const float* Matrix3x3::Row(int row) const {
        return elements[row].data();
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
        std::array<float, 16> flat = {
            elements[0][0], elements[0][1], elements[0][2], 0.0f,
            elements[1][0], elements[1][1], elements[1][2], 0.0f,
            elements[2][0], elements[2][1], elements[2][2], 0.0f,
            0.0f,           0.0f,           0.0f,           1.0f
        };
        return Matrix4x4(flat);
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
