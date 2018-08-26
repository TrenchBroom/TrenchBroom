/*
 Copyright (C) 2010-2017 Kristian Duske
 
 This file is part of TrenchBroom.
 
 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 TrenchBroom is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TrenchBroom_Mat_h
#define TrenchBroom_Mat_h

#include "Quat.h"
#include "vec_type.h"

#include <algorithm>
#include <cassert>
#include <iterator>

template <typename T, size_t R, size_t C>
class Mat {
public:
    typedef T Type;
    static const size_t Rows = R;
    static const size_t Cols = C;
    
    static const Mat<T,R,C> Null;
    static const Mat<T,R,C> Identity;
    static const Mat<T,R,C> Rot90XCW;
    static const Mat<T,R,C> Rot90YCW;
    static const Mat<T,R,C> Rot90ZCW;
    static const Mat<T,R,C> Rot90XCCW;
    static const Mat<T,R,C> Rot90YCCW;
    static const Mat<T,R,C> Rot90ZCCW;
    static const Mat<T,R,C> Rot180X;
    static const Mat<T,R,C> Rot180Y;
    static const Mat<T,R,C> Rot180Z;
    static const Mat<T,R,C> MirX;
    static const Mat<T,R,C> MirY;
    static const Mat<T,R,C> MirZ;
    static const Mat<T,R,C> ZerX;
    static const Mat<T,R,C> ZerY;
    static const Mat<T,R,C> ZerZ;
    static const Mat<T,R,C> YIQToRGB;
    static const Mat<T,R,C> RGBToYIQ;
    
    typedef std::vector<Mat<T,R,C> > List;
    
    // we store in column-major format
    // every vector is one column
    vec<T,R> v[C];
    
    Mat<T,R,C>() {
        setIdentity();
    }
    
    // Copy and move constructors
    Mat(const Mat<T,R,C>& other) = default;
    Mat(Mat<T,R,C>&& other) = default;
    
    // Assignment operators
    Mat<T,R,C>& operator=(const Mat<T,R,C>& other) = default;
    Mat<T,R,C>& operator=(Mat<T,R,C>&& other) = default;
    
    // Conversion constructor
    template <typename U>
    Mat(const Mat<U,R,C>& other) {
        for (size_t c = 0; c < C; ++c) {
            for (size_t r = 0; r < R; ++r) {
                v[c][r] = static_cast<T>(other[c][r]);
            }
        }
    }

    Mat<T,R,C>(const T v11, const T v12, const T v13,
               const T v21, const T v22, const T v23,
               const T v31, const T v32, const T v33) {
        v[0][0] = v11; v[1][0] = v12; v[2][0] = v13;
        v[0][1] = v21; v[1][1] = v22; v[2][1] = v23;
        v[0][2] = v31; v[1][2] = v32; v[2][2] = v33;
        for (size_t c = 3; c < C; c++)
            for (size_t r = 3; r < R; r++)
                v[c][r] = static_cast<T>(0.0);
    }
    
    Mat<T,R,C>(const T v11, const T v12, const T v13, const T v14,
               const T v21, const T v22, const T v23, const T v24,
               const T v31, const T v32, const T v33, const T v34,
               const T v41, const T v42, const T v43, const T v44) {
        v[0][0] = v11; v[1][0] = v12; v[2][0] = v13; v[3][0] = v14;
        v[0][1] = v21; v[1][1] = v22; v[2][1] = v23; v[3][1] = v24;
        v[0][2] = v31; v[1][2] = v32; v[2][2] = v33; v[3][2] = v34;
        v[0][3] = v41; v[1][3] = v42; v[2][3] = v43; v[3][3] = v44;
        for (size_t c = 4; c < C; c++)
            for (size_t r = 4; r < R; r++)
                v[c][r] = static_cast<T>(0.0);
    }
    
    const Mat<T,R,C> operator-() const {
        Mat<T,R,C> result;
        for (size_t c = 0; c < C; c++)
            result[c] = -v[c];
        return result;
    }
    
    bool operator==(const Mat<T,R,C>& right) const {
        for (size_t c = 0; c < C; c++)
            for (size_t r = 0; r < R; r++)
                if (v[c][r] != right[c][r])
                    return false;
        return true;
    }
    
    // Matrix addition and subtraction
    const Mat<T,R,C> operator+(const Mat<T,R,C>& right) const {
        Mat<T,R,C> result(*this);
        return result += right;
    }
    
    Mat<T,R,C>& operator+= (const Mat<T,R,C>& right) {
        for (size_t c = 0; c < C; c++)
            v[c] += right[c];
        return *this;
    }
    
    const Mat<T,R,C> operator-(const Mat<T,R,C>& right) const {
        Mat<T,R,C> result(*this);
        return result -= right;
    }
    
    Mat<T,R,C>& operator-= (const Mat<T,R,C>& right) {
        for (size_t c = 0; c < C; c++)
            v[c] -= right[c];
        return *this;
    }
    
    // Matrix multiplication
    const Mat<T,R,C> operator*(const Mat<T,C,R>& right) const {
        Mat<T,R,C> result(Mat<T,R,C>::Null);
        for (size_t c = 0; c < C; c++)
            for (size_t r = 0; r < R; r++)
                for (size_t i = 0; i < C; ++i)
                    result[c][r] += v[i][r] * right[c][i];
        return result;
    }
    
    Mat<T,R,C>& operator*= (const Mat<T,C,R>& right) {
        return *this = *this * right;
    }
    
    // Scalar multiplication
    const Mat<T,R,C> operator*(const T right) const {
        Mat<T,R,C> result(*this);
        return result *= right;
    }
    
    Mat<T,R,C>& operator*= (const T right) {
        for (size_t c = 0; c < C; c++)
            v[c] *= right;
        return *this;
    }
    
    const Mat<T,R,C> operator/(const T right) const {
        Mat<T,R,C> result(*this);
        return result /= right;
    }
    
    Mat<T,R,C>& operator/= (const T right) {
        for (size_t c = 0; c < C; c++)
            v[c] /= right;
        return *this;
    }
    
    // Vector right multiplication
    const vec<T,C> operator*(const vec<T,C>& right) const {
        vec<T,C> result;
        for (size_t r = 0; r < R; r++)
            for (size_t c = 0; c < C; ++c)
                result[r] += v[c][r] * right[c];
        return result;
    }
    
    const vec<T,C-1> operator*(const vec<T,C-1>& right) const {
        return toCartesianCoords(*this * toHomogeneousCoords(right));
    }
    
    // Vector list right multiplication
    const typename vec<T,C>::List operator*(const typename vec<T,C>::List& right) const {
        typename vec<T,C>::List result;
        result.reserve(right.size());
        std::transform(std::begin(right), std::end(right), std::back_inserter(result), [this](const vec<T,C>& elem) { return *this * elem; });
        return result;
    }
    
    const typename vec<T,C-1>::List operator*(const typename vec<T,C-1>::List& right) const {
        typename vec<T,C-1>::List result;
        result.reserve(right.size());
        std::transform(std::begin(right), std::end(right), std::back_inserter(result), [this](const vec<T,C-1>& elem) { return *this * elem; });
        return result;
    }
    
    // indexed access, returns one column
    vec<T,R>& operator[] (const size_t index) {
        assert(index < C);
        return v[index];
    }
    
    const vec<T,R>& operator[] (const size_t index) const {
        assert(index < C);
        return v[index];
    }
    
    bool equals(const Mat<T,R,C>& other, const T epsilon = Math::Constants<T>::almostZero()) const {
        for (size_t c = 0; c < C; c++) {
            if (!equal(v[c], other[c], epsilon)) {
                return false;
            }
        }
        return true;
    }
    
    bool null() const {
        return equals(Null);
    }
    
    Mat<T,R,C>& setIdentity() {
        for (size_t c = 0; c < C; c++)
            for (size_t r = 0; r < R; r++)
                v[c][r] = c == r ? static_cast<T>(1.0) : static_cast<T>(0.0);
        return *this;
    }
    
    Mat<T,R,C>& setNull() {
        for (size_t c = 0; c < C; c++)
            for (size_t r = 0; r < R; r++)
                v[c][r] = static_cast<T>(0.0);
        return *this;
    }
    
    const Mat<T,C,R> transposed() const {
        Mat<T,C,R> result;
        for (size_t c = 0; c < C; c++)
            for (size_t r = 0; r < R; r++)
                result[r][c] = v[c][r];
        return result;
    }
    
    void write(T* buffer) const {
        for (size_t c = 0; c < C; c++)
            for (size_t r = 0; r < R; r++)
                buffer[(c*C + r)] = v[c][r];
    }
};

template <typename T, size_t R, size_t C>
Mat<T,R,C> operator*(const T left, const Mat<T,R,C>& right) {
    return right * left;
}

// Vector left multiplication with vector of dimension R
template <typename T, size_t R, size_t C>
const vec<T,R> operator*(const vec<T,R>& left, const Mat<T,R,C>& right) {
    vec<T,R> result;
    for (size_t c = 0; c < C; c++)
        result[c] = dot(left, right[c]);
    return result;
}

template <typename T, size_t R, size_t C>
vec<T,R>& operator*= (vec<T,R>& left, const Mat<T,R,C>& right) {
    return left = left * right;
}

// Vector left multiplication with list of vectors of dimension R
template <typename T, size_t R, size_t C>
const typename vec<T,R>::List operator*(const typename vec<T,R>::List& left, const Mat<T,R,C>& right) {
    typename vec<T,R>::List result;
    result.reserve(left.size());
    std::transform(std::begin(left), std::end(left), std::back_inserter(result), [right](const vec<T,R>& elem) { return elem * right; });
    return result;
}

template <typename T, size_t R, size_t C>
const typename vec<T,R>::List& operator*= (typename vec<T,R>::List& left, const Mat<T,R,C>& right) {
    for (vec<T,R>& elem : left)
        elem *= right;
    return left;
}

// Vector left multiplication with vector of dimension R-1
template <typename T, size_t R, size_t C>
const vec<T,R-1> operator*(const vec<T,R-1>& left, const Mat<T,R,C>& right) {
    return toCartesianCoords(toHomogeneousCoords(left) * right);
}

template <typename T, size_t R, size_t C>
vec<T,R-1>& operator*= (vec<T,R-1>& left, const Mat<T,R,C>& right) {
    return left = left * right;
}

// Vector left multiplication with list of vectors of dimension R-1
template <typename T, size_t R, size_t C>
const typename vec<T,R-1>::List operator*(const typename vec<T,R-1>::List& left, const Mat<T,R,C>& right) {
    typename vec<T,R-1>::List result;
    result.reserve(left.size());
    std::transform(std::begin(left), std::end(left), std::back_inserter(result), [right](const vec<T,R-1>& elem) { return elem * right; });
    return result;
}

template <typename T, size_t R, size_t C>
typename vec<T,R-1>::List& operator*= (typename vec<T,R-1>::List& left, const Mat<T,R,C>& right) {
    for (vec<T,R-1>& elem : left)
        elem *= right;
    return left;
}

template <typename T, size_t S>
Mat<T,S,S>& transposeMatrix(Mat<T,S,S>& mat) {
    using std::swap;
    for (size_t c = 0; c < S; c++)
        for (size_t r = c + 1; r < S; r++)
            swap(mat[c][r], mat[r][c]);
    return mat;
}

template <typename T, size_t S>
const Mat<T,S-1,S-1> minorMatrix(const Mat<T,S,S>& mat, const size_t row, const size_t col) {
    Mat<T,S-1,S-1> min;
    size_t minC, minR;
    minC = 0;
    for (size_t c = 0; c < S; c++) {
        if (c != col) {
            minR = 0;
            for (size_t r = 0; r < S; r++)
                if (r != row)
                    min[minC][minR++] = mat[c][r];
            minC++;
        }
    }
    return min;
}

template <typename T, size_t S>
struct MatrixDeterminant {
    T operator() (const Mat<T,S,S>& mat) const {
        // Laplace after first col
        T det = static_cast<T>(0.0);
        for (size_t r = 0; r < S; r++) {
            const T f = static_cast<T>(r % 2 == 0 ? 1.0 : -1.0);
            det += f * mat[0][r] * MatrixDeterminant<T,S-1>()(minorMatrix(mat, r, 0));
        }
        return det;
    }
};

// TODO: implement faster block-matrix based method for NxN matrices where N = 2^n

template <typename T>
struct MatrixDeterminant<T,3> {
    T operator() (const Mat<T,3,3>& mat) const {
        return (  mat[0][0]*mat[1][1]*mat[2][2]
                + mat[1][0]*mat[2][1]*mat[0][2]
                + mat[2][0]*mat[0][1]*mat[1][2]
                - mat[2][0]*mat[1][1]*mat[0][2]
                - mat[1][0]*mat[0][1]*mat[2][2]
                - mat[0][0]*mat[2][1]*mat[1][2]);
    }
};

template <typename T>
struct MatrixDeterminant<T,2> {
    T operator() (const Mat<T,2,2>& mat) const {
        return mat[0][0]*mat[1][1] - mat[1][0]*mat[0][1];
    }
};

template <typename T>
struct MatrixDeterminant<T,1> {
    T operator() (const Mat<T,1,1>& mat) const {
        return mat[0][0];
    }
};

template <typename T, size_t S>
T matrixDeterminant(const Mat<T,S,S>& mat) {
    return MatrixDeterminant<T,S>()(mat);
}

template <typename T, size_t S>
Mat<T,S,S>& adjoinMatrix(Mat<T,S,S>& mat) {
    mat = adjointMatrix(mat);
    return mat;
}

template <typename T, size_t S>
Mat<T,S,S> adjointMatrix(const Mat<T,S,S>& mat) {
    Mat<T,S,S> result;
    for (size_t c = 0; c < S; c++) {
        for (size_t r = 0; r < S; r++) {
            const T f = static_cast<T>((c + r) % 2 == 0 ? 1.0 : -1.0);
            result[r][c] = f * matrixDeterminant(minorMatrix(mat, r, c)); // transpose the matrix on the fly
        }
    }
    return result;
}

template <typename T, size_t S>
Mat<T,S,S>& invertMatrix(Mat<T,S,S>& mat, bool& invertible) {
    mat = invertedMatrix(mat, invertible);
    return mat;
}

template <typename T, size_t S>
Mat<T,S,S>& invertMatrix(Mat<T,S,S>& mat) {
    bool invertible = true;
    invertMatrix(mat, invertible);
    assert(invertible);
    return mat;
}

template <typename T, size_t S>
Mat<T,S,S> invertedMatrix(const Mat<T,S,S>& mat, bool& invertible) {
    const T det = matrixDeterminant(mat);
    invertible = det != 0.0;
    if (!invertible)
        return mat;
    
    return adjointMatrix(mat) / det;
}

template <typename T, size_t S>
Mat<T,S,S> invertedMatrix(const Mat<T,S,S>& mat) {
    bool invertible = true;
    const Mat<T,S,S> inverted = invertedMatrix(mat, invertible);
    assert(invertible);
    return inverted;
}

template <typename T>
Mat<T,4,4> perspectiveMatrix(const T fov, const T nearPlane, const T farPlane, const int width, const int height) {
    const T vFrustum = std::tan(Math::radians(fov) / static_cast<T>(2.0)) * static_cast<T>(0.75) * nearPlane;
    const T hFrustum = vFrustum * static_cast<T>(width) / static_cast<T>(height);
    const T depth = farPlane - nearPlane;
    
    static const T zero = static_cast<T>(0.0);
    static const T one  = static_cast<T>(1.0);
    static const T two  = static_cast<T>(2.0);
    
    return Mat<T,4,4>(nearPlane / hFrustum, zero,                    zero,                               zero,
                      zero,                 nearPlane / vFrustum,    zero,                               zero,
                      zero,                 zero,                   -(farPlane + nearPlane) / depth,    -two * farPlane * nearPlane / depth,
                      zero,                 zero,                   -one,                                zero);
}

template <typename T>
Mat<T,4,4> orthoMatrix(const T nearPlane, const T farPlane, const T left, const T top, const T right, const T bottom) {
    const T width = right - left;
    const T height = top - bottom;
    const T depth = farPlane - nearPlane;
    
    static const T zero = static_cast<T>(0.0);
    static const T one  = static_cast<T>(1.0);
    static const T two  = static_cast<T>(2.0);
    
    return Mat<T,4,4>(two / width,  zero,            zero,          -(left + right) / width,
                      zero,         two / height,    zero,          -(top + bottom) / height,
                      zero,         zero,           -two / depth,   -(farPlane + nearPlane) / depth,
                      zero,         zero,            zero,           one);
}

template <typename T>
Mat<T,4,4> viewMatrix(const vec<T,3>& direction, const vec<T,3>& up) {
    const vec<T,3>& f = direction;
    const vec<T,3> s = cross(f, up);
    const vec<T,3> u = cross(s, f);
    
    static const T zero = static_cast<T>(0.0);
    static const T one  = static_cast<T>(1.0);
    
    return Mat<T,4,4>( s[0],  s[1],  s[2], zero,
                      u[0],  u[1],  u[2], zero,
                      -f[0], -f[1], -f[2], zero,
                      zero,  zero,  zero, one);
}

/**
 Returns a matrix that will rotate any point counter-clockwise about the given angles (in radians).
 */
template <typename T>
Mat<T,4,4> rotationMatrix(const T roll, const T pitch, const T yaw) {
    static const T I = static_cast<T>(1.0);
    static const T O = static_cast<T>(0.0);
    
    const T Cr = std::cos(roll);
    const T Sr = std::sin(roll);
    const  Mat<T,4,4> R( +I,  +O,  +O,  +O,
                         +O, +Cr, -Sr,  +O,
                         +O, +Sr, +Cr,  +O,
                         +O,  +O,  +O,  +I);
    
    const T Cp = std::cos(pitch);
    const T Sp = std::sin(pitch);
    const Mat<T,4,4> P(+Cp,  +O, +Sp,  +O,
                        +O,  +I,  +O,  +O,
                       -Sp,  +O, +Cp,  +O,
                        +O,  +O,  +O,  +I);
    
    const T Cy = std::cos(yaw);
    const T Sy = std::sin(yaw);
    const Mat<T,4,4> Y(+Cy, -Sy,  +O,  +O,
                       +Sy, +Cy,  +O,  +O,
                        +O,  +O,  +I,  +O,
                        +O,  +O,  +O,  +I);
    
    return Y * P * R;
}

/**
 Returns a matrix that will rotate any point counter-clockwise about the given angles (in radians).
 */
template <typename T>
Mat<T,4,4> rotationMatrix(const vec<T,3>& a) {
    return rotationMatrix(a.x(), a.y(), a.z());
}

/**
 Returns a matrix that will rotate any point counter-clockwise about the given axis by the given angle (in radians).
 */
template <typename T>
Mat<T,4,4> rotationMatrix(const vec<T,3>& axis, const T angle) {
    const T s = std::sin(-angle);
    const T c = std::cos(-angle);
    const T i = static_cast<T>(1.0 - c);
    
    const T ix  = i  * axis[0];
    const T ix2 = ix * axis[0];
    const T ixy = ix * axis[1];
    const T ixz = ix * axis[2];
    
    const T iy  = i  * axis[1];
    const T iy2 = iy * axis[1];
    const T iyz = iy * axis[2];
    
    const T iz2 = i  * axis[2] * axis[2];
    
    const T sx = s * axis[0];
    const T sy = s * axis[1];
    const T sz = s * axis[2];
    
    Mat<T,4,4> rotation;
    rotation[0][0] = ix2 + c;
    rotation[0][1] = ixy - sz;
    rotation[0][2] = ixz + sy;
    
    rotation[1][0] = ixy + sz;
    rotation[1][1] = iy2 + c;
    rotation[1][2] = iyz - sx;
    
    rotation[2][0] = ixz - sy;
    rotation[2][1] = iyz + sx;
    rotation[2][2] = iz2 + c;
    
    return rotation;
}

template <typename T>
Mat<T,4,4> rotationMatrix(const Quat<T>& quat) {
    // see http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToMatrix/
    
    const T x = quat.v[0];
    const T y = quat.v[1];
    const T z = quat.v[2];
    const T w = quat.r;
    
    const T x2 = x*x;
    const T y2 = y*y;
    const T z2 = z*z;
    
    Mat<T,4,4> rotation;
    rotation[0][0] = static_cast<T>(1.0 - 2.0*(y2 + z2));// a2 + b2 - c2 - d2;
    rotation[0][1] = static_cast<T>(2.0*(x*y + z*w));
    rotation[0][2] = static_cast<T>(2.0*(x*z - y*w));
    
    rotation[1][0] = static_cast<T>(2.0*(x*y - z*w));
    rotation[1][1] = static_cast<T>(1.0 - 2.0*(x2 + z2));//a2 - b2 + c2 - d2;
    rotation[1][2] = static_cast<T>(2.0*(y*z + x*w));
    
    rotation[2][0] = static_cast<T>(2.0*(x*z + y*w));
    rotation[2][1] = static_cast<T>(2.0*(y*z - x*w));
    rotation[2][2] = static_cast<T>(1.0 - 2.0*(x2 + y2));// a2 - b2 - c2 + d2;
    
    return rotation;
}

/**
 Returns a matrix that will rotate the given from vector onto the given to vector
 about their perpendicular axis. The vectors are expected to be normalized.
 */
template <typename T>
Mat<T,4,4> rotationMatrix(const vec<T,3>& from, const vec<T,3>& to) {
    return rotationMatrix(Quat<T>(from, to));
}

template <typename T, size_t S>
Mat<T,S+1,S+1> translationMatrix(const vec<T,S>& delta) {
    Mat<T,S+1,S+1> translation;
    for (size_t i = 0; i < S; ++i)
        translation[S][i] = delta[i];
    return translation;
}

template <typename T, size_t S>
Mat<T,S,S> translationMatrix(const Mat<T,S,S>& mat) {
    Mat<T,S,S> result;
    for (size_t i = 0; i < S-1; ++i)
        result[S-1][i] = mat[S-1][i];
    return result;
}

template <typename T, size_t S>
Mat<T,S,S> stripTranslation(const Mat<T,S,S>& mat) {
    Mat<T,S,S> result(mat);
    for (size_t i = 0; i < S-1; ++i)
        result[S-1][i] = static_cast<T>(0.0);
    return result;
}

template <typename T, size_t S>
Mat<T,S+1,S+1> scalingMatrix(const vec<T,S>& factors) {
    Mat<T,S+1,S+1> scaling;
    for (size_t i = 0; i < S; ++i)
        scaling[i][i] = factors[i];
    return scaling;
}

template <size_t S, typename T>
Mat<T,S,S> scalingMatrix(const T f) {
    Mat<T,S,S> scaling;
    for (size_t i = 0; i < S-1; ++i)
        scaling[i][i] = f;
    return scaling;
}

template <typename T>
const Mat<T,4,4>& mirrorMatrix(const Math::Axis::Type axis) {
    switch (axis) {
        case Math::Axis::AX:
            return Mat<T,4,4>::MirX;
        case Math::Axis::AY:
            return Mat<T,4,4>::MirY;
        case Math::Axis::AZ:
            return Mat<T,4,4>::MirZ;
        default:
            return Mat<T,4,4>::Identity;
    }
}

template <typename T>
Mat<T,4,4> coordinateSystemMatrix(const vec<T,3>& x, const vec<T,3>& y, const vec<T,3>& z, const vec<T,3>& o) {
    return invertedMatrix(Mat<T,4,4>(x[0], y[0], z[0], o[0],
                                     x[1], y[1], z[1], o[1],
                                     x[2], y[2], z[2], o[2],
                                     0.0,  0.0,  0.0,  1.0));
}

/**
 Returns a matrix that will transform a point to a coordinate system where the X and
 Y axes are in the given plane and the Z axis is parallel to the given direction. This is useful for
 projecting points onto a plane along a particular direction.
 */
template <typename T>
Mat<T,4,4> planeProjectionMatrix(const T distance, const vec<T,3>& normal, const vec<T,3>& direction) {
    // create some coordinate system where the X and Y axes are contained within the plane
    // and the Z axis is the projection direction
    vec<T,3> xAxis;
    
    switch (firstComponent(normal)) {
        case Math::Axis::AX:
            xAxis = normalize(cross(normal, vec<T, 3>::pos_z));
            break;
        default:
            xAxis = normalize(cross(normal, vec<T, 3>::pos_x));
            break;
    }
    const vec<T,3>  yAxis = normalize(cross(normal, xAxis));
    const vec<T,3>& zAxis = direction;
    
    assert(Math::eq(length(xAxis), 1.0));
    assert(Math::eq(length(yAxis), 1.0));
    assert(Math::eq(length(zAxis), 1.0));
    
    return coordinateSystemMatrix(xAxis, yAxis, zAxis, distance * normal);
}

/**
 Returns the inverse of a matrix that will transform a point to a coordinate system where the X and
 Y axes are in the given plane and the Z axis is the given normal.
 */
template <typename T>
Mat<T,4,4> planeProjectionMatrix(const T distance, const vec<T,3>& normal) {
    return planeProjectionMatrix(distance, normal, normal);
}

/**
 Returns a matrix that rotates a 3D vector in counter clockwise direction about the Z axis by the given angle
 (in radians).
 */
template <typename T>
Mat<T,3,3> rotationMatrix(const T angle) {
    const T sin = std::sin(angle);
    const T cos = std::cos(angle);
    return Mat<T,3,3>(cos, -sin, 0.0,
                      sin,  cos, 0.0,
                      0.0,  0.0, 1.0);
}

/**
From: http://web.archive.org:80/web/20041029003853/http://www.j3d.org/matrix_faq/matrfaq_latest.html#Q43
*/
template <typename T>
Mat<T,4,4> shearMatrix(const T Sxy, const T Sxz, const T Syx, const T Syz, const T Szx, const T Szy) {
    return Mat<T,4,4>(1.0, Syx, Szx, 0.0,
                      Sxy, 1.0, Szy, 0.0,
                      Sxz, Syz, 1.0, 0.0,
                      0.0, 0.0, 0.0, 1.0);
}


template <typename T, size_t R, size_t C>
const Mat<T,R,C> Mat<T,R,C>::Identity = Mat<T,R,C>().setIdentity();

template <typename T, size_t R, size_t C>
const Mat<T,R,C> Mat<T,R,C>::Null = Mat<T,R,C>().setNull();

template <typename T, size_t R, size_t C>
const Mat<T,R,C> Mat<T,R,C>::Rot90XCW    = Mat<T,R,C>(+static_cast<T>(1.0), +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(0.0),
                                                      +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(1.0), +static_cast<T>(0.0),
                                                      +static_cast<T>(0.0), -static_cast<T>(1.0), +static_cast<T>(0.0), +static_cast<T>(0.0),
                                                      +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(1.0));
template <typename T, size_t R, size_t C>
const Mat<T,R,C> Mat<T,R,C>::Rot90YCW    = Mat<T,R,C>(+static_cast<T>(0.0), +static_cast<T>(0.0), -static_cast<T>(1.0), +static_cast<T>(0.0),
                                                      +static_cast<T>(0.0), +static_cast<T>(1.0), +static_cast<T>(0.0), +static_cast<T>(0.0),
                                                      +static_cast<T>(1.0), +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(0.0),
                                                      +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(1.0));
template <typename T, size_t R, size_t C>
const Mat<T,R,C> Mat<T,R,C>::Rot90ZCW    = Mat<T,R,C>(+static_cast<T>(0.0), +static_cast<T>(1.0), +static_cast<T>(0.0), +static_cast<T>(0.0),
                                                      -static_cast<T>(1.0), +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(0.0),
                                                      +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(1.0), +static_cast<T>(0.0),
                                                      +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(1.0));
template <typename T, size_t R, size_t C>
const Mat<T,R,C> Mat<T,R,C>::Rot90XCCW   = Mat<T,R,C>(+static_cast<T>(1.0), +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(0.0),
                                                      +static_cast<T>(0.0), +static_cast<T>(0.0), -static_cast<T>(1.0), +static_cast<T>(0.0),
                                                      +static_cast<T>(0.0), +static_cast<T>(1.0), +static_cast<T>(0.0), +static_cast<T>(0.0),
                                                      +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(1.0));
template <typename T, size_t R, size_t C>
const Mat<T,R,C> Mat<T,R,C>::Rot90YCCW   = Mat<T,R,C>(+static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(1.0), +static_cast<T>(0.0),
                                                      +static_cast<T>(0.0), +static_cast<T>(1.0), +static_cast<T>(0.0), +static_cast<T>(0.0),
                                                      -static_cast<T>(1.0), +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(0.0),
                                                      +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(1.0));
template <typename T, size_t R, size_t C>
const Mat<T,R,C> Mat<T,R,C>::Rot90ZCCW   = Mat<T,R,C>(+static_cast<T>(0.0), -static_cast<T>(1.0), +static_cast<T>(0.0), +static_cast<T>(0.0),
                                                      +static_cast<T>(1.0), +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(0.0),
                                                      +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(1.0), +static_cast<T>(0.0),
                                                      +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(1.0));
template <typename T, size_t R, size_t C>
const Mat<T,R,C> Mat<T,R,C>::Rot180X     = Mat<T,R,C>(+static_cast<T>(1.0), +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(0.0),
                                                      +static_cast<T>(0.0), -static_cast<T>(1.0), +static_cast<T>(0.0), +static_cast<T>(0.0),
                                                      +static_cast<T>(0.0), +static_cast<T>(0.0), -static_cast<T>(1.0), +static_cast<T>(0.0),
                                                      +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(1.0));
template <typename T, size_t R, size_t C>
const Mat<T,R,C> Mat<T,R,C>::Rot180Y     = Mat<T,R,C>(-static_cast<T>(1.0), +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(0.0),
                                                      +static_cast<T>(0.0), +static_cast<T>(1.0), +static_cast<T>(0.0), +static_cast<T>(0.0),
                                                      +static_cast<T>(0.0), +static_cast<T>(0.0), -static_cast<T>(1.0), +static_cast<T>(0.0),
                                                      +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(1.0));
template <typename T, size_t R, size_t C>
const Mat<T,R,C> Mat<T,R,C>::Rot180Z     = Mat<T,R,C>(-static_cast<T>(1.0), +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(0.0),
                                                      +static_cast<T>(0.0), -static_cast<T>(1.0), +static_cast<T>(0.0), +static_cast<T>(0.0),
                                                      +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(1.0), +static_cast<T>(0.0),
                                                      +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(1.0));
template <typename T, size_t R, size_t C>
const Mat<T,R,C> Mat<T,R,C>::MirX        = Mat<T,R,C>(-static_cast<T>(1.0), +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(0.0),
                                                      +static_cast<T>(0.0), +static_cast<T>(1.0), +static_cast<T>(0.0), +static_cast<T>(0.0),
                                                      +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(1.0), +static_cast<T>(0.0),
                                                      +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(1.0));
template <typename T, size_t R, size_t C>
const Mat<T,R,C> Mat<T,R,C>::MirY        = Mat<T,R,C>(+static_cast<T>(1.0), +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(0.0),
                                                      +static_cast<T>(0.0), -static_cast<T>(1.0), +static_cast<T>(0.0), +static_cast<T>(0.0),
                                                      +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(1.0), +static_cast<T>(0.0),
                                                      +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(1.0));
template <typename T, size_t R, size_t C>
const Mat<T,R,C> Mat<T,R,C>::MirZ        = Mat<T,R,C>(+static_cast<T>(1.0), +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(0.0),
                                                      +static_cast<T>(0.0), +static_cast<T>(1.0), +static_cast<T>(0.0), +static_cast<T>(0.0),
                                                      +static_cast<T>(0.0), +static_cast<T>(0.0), -static_cast<T>(1.0), +static_cast<T>(0.0),
                                                      +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(1.0));
template <typename T, size_t R, size_t C>
const Mat<T,R,C> Mat<T,R,C>::ZerX        = Mat<T,R,C>(+static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(0.0),
                                                      +static_cast<T>(0.0), +static_cast<T>(1.0), +static_cast<T>(0.0), +static_cast<T>(0.0),
                                                      +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(1.0), +static_cast<T>(0.0),
                                                      +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(1.0));
template <typename T, size_t R, size_t C>
const Mat<T,R,C> Mat<T,R,C>::ZerY        = Mat<T,R,C>(+static_cast<T>(1.0), +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(0.0),
                                                      +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(0.0),
                                                      +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(1.0), +static_cast<T>(0.0),
                                                      +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(1.0));
template <typename T, size_t R, size_t C>
const Mat<T,R,C> Mat<T,R,C>::ZerZ        = Mat<T,R,C>(+static_cast<T>(1.0), +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(0.0),
                                                      +static_cast<T>(0.0), +static_cast<T>(1.0), +static_cast<T>(0.0), +static_cast<T>(0.0),
                                                      +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(0.0),
                                                      +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(1.0));
template <typename T, size_t R, size_t C>
const Mat<T,R,C> Mat<T,R,C>::YIQToRGB     = Mat<T,R,C>(+static_cast<T>(1.0),+static_cast<T>( 0.9563),+static_cast<T> (0.6210),
                                                       +static_cast<T>(1.0),+static_cast<T>(-0.2721),+static_cast<T>(-0.6474),
                                                       +static_cast<T>(1.0),+static_cast<T>(-1.1070),+static_cast<T>( 1.7046));

template <typename T, size_t R, size_t C>
const Mat<T,R,C> Mat<T,R,C>::RGBToYIQ     = Mat<T,R,C>(+static_cast<T>(0.299),   +static_cast<T>( 0.587),   +static_cast<T>( 0.114),
                                                       +static_cast<T>(0.595716),+static_cast<T>(-0.274453),+static_cast<T>(-0.321263),
                                                       +static_cast<T>(0.211456),+static_cast<T>(-0.522591),+static_cast<T>( 0.311135));


typedef Mat<float,2,2> Mat2x2f;
typedef Mat<float,3,3> Mat3x3f;
typedef Mat<float,4,4> Mat4x4f;
typedef Mat<double,2,2> Mat2x2d;
typedef Mat<double,3,3> Mat3x3d;
typedef Mat<double,4,4> Mat4x4d;

#endif
