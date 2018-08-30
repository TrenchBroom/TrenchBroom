vec/*
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

#ifndef TRENCHBROOM_MAT_FUNCTIONS_H
#define TRENCHBROOM_MAT_FUNCTIONS_H

#include "mat_type.h"
#include "vec_type.h"
#include "vec_functions.h"
#include "Quat.h"

#include <cassert>

/**
 * Compares the given two matrices column wise.
 *
 * @tparam T the component type
 * @tparam R the number of rows
 * @tparam C the number of columns
 * @param lhs the first matrix
 * @param rhs the second matrix
 * @param epsilon the epsilon value
 * @return a negative value if there is a column in the left matrix that compares less than its corresponding
 * column of the right matrix, a positive value in the opposite case, and 0 if all columns compare equal
 */
template <typename T, size_t R, size_t C>
int compare(const mat<T,R,C>& lhs, const mat<T,R,C>& rhs, const T epsilon) {
    for (size_t c = 0; c < C; c++) {
        const auto cmp = compare(lhs[c], rhs[c], epsilon);
        if (cmp != 0) {
            return cmp;
        }
    }
    return 0;
}

/**
 * Checks whether the given matrices have identical components.
 *
 * @tparam T the component type
 * @tparam R the number of rows
 * @tparam C the number of columns
 * @param lhs the first matrix
 * @param rhs the second matrix
 * @return true if all components of the given matrices are equal, and false otherwise
 */
template <typename T, size_t R, size_t C>
bool operator==(const mat<T,R,C>& lhs, const mat<T,R,C>& rhs) {
    return compare(lhs, rhs, static_cast<T>(0.0)) == 0;
}

/**
 * Checks whether the given matrices have identical components.
 *
 * @tparam T the component type
 * @tparam R the number of rows
 * @tparam C the number of columns
 * @param lhs the first matrix
 * @param rhs the second matrix
 * @return false if all components of the given matrices are equal, and true otherwise
 */
template <typename T, size_t R, size_t C>
bool operator!=(const mat<T,R,C>& lhs, const mat<T,R,C>& rhs) {
    return compare(lhs, rhs, static_cast<T>(0.0)) != 0;
}

/**
 * Checks whether the given matrices have equal components.
 *
 * @tparam T the component type
 * @tparam R the number of rows
 * @tparam C the number of columns
 * @param lhs the first matrix
 * @param rhs the second matrix
 * @param epsilon the epsilon value
 * @return true if all components of the given matrices are equal, and false otherwise
 */
template <typename T, size_t R, size_t C>
bool equal(const mat<T,R,C>& lhs, const mat<T,R,C>& rhs, const T epsilon) {
    return compare(lhs, rhs, epsilon) == 0;
}

/**
 * Checks whether all columns of the given matrix are zero.
 *
 * @tparam T the component type
 * @tparam R the number of rows
 * @tparam C the number of columns
 * @param m the matrix to check
 * @param epsilon the epsilon value
 * @return true if all columsn of the given matrix are zero
 */
template <typename T, size_t R, size_t C>
bool isZero(const mat<T,R,C>& m, const T epsilon = Math::Constants<T>::almostZero()) {
    for (size_t c = 0; c < C; ++c) {
        if (!isZero(m[c], epsilon)) {
            return false;
        }
    }
    return true;
}

/**
 * Returns a matrix with the negated components of the given matrix.
 *
 * @param m the matrix to negate
 * @return the negated matrix
 */
template <typename T, size_t R, size_t C>
mat<T,R,C> operator-(const mat<T,R,C>& m) {
    mat<T,R,C> result;
    for (size_t c = 0; c < C; c++) {
        result[c] = -m[c];
    }
    return result;
}

/**
 * Computes the sum of two matrices by adding the corresponding components.
 *
 * @tparam T the component type
 * @tparam R the number of rows
 * @tparam C the number of columns
 * @param lhs the first matrix
 * @param rhs the second matrix
 * @return a matrix where each component is the sum of the two corresponding components of the given matrices
 */
template <typename T, size_t R, size_t C>
mat<T,R,C> operator+(const mat<T,R,C>& lhs, const mat<T,R,C>& rhs) {
    mat<T,R,C> result;
    for (size_t c = 0; c < C; c++) {
        result[c] = lhs[c] + rhs[c];
    }
    return result;
}

/**
 * Computes the difference of two matrices by subtracting the corresponding components.
 *
 * @tparam T the component type
 * @tparam R the number of rows
 * @tparam C the number of columns
 * @param lhs the first matrix
 * @param rhs the second matrix
 * @return a matrix where each component is the difference of the two corresponding components of the given matrices
 */
template <typename T, size_t R, size_t C>
mat<T,R,C> operator-(const mat<T,R,C>& lhs, const mat<T,R,C>& rhs) {
    mat<T,R,C> result;
    for (size_t c = 0; c < C; c++) {
        result[c] = lhs[c] - rhs[c];
    }
    return result;
}

/**
 * Computes the product of the given two matrices.
 *
 * @tparam T the component type
 * @tparam R the number of rows
 * @tparam C the number of columns
 * @param lhs the first matrix
 * @param rhs the second matrix
 * @return the product of the given matrices
 */
template <typename T, size_t R, size_t C>
mat<T,R,C> operator*(const mat<T,C,R>& lhs, const mat<T,C,R>& rhs) {
    auto result = mat<T,R,C>::zero;
    for (size_t c = 0; c < C; c++) {
        for (size_t r = 0; r < R; r++) {
            for (size_t i = 0; i < C; ++i) {
                result[c][r] += lhs[i][r] * rhs[c][i];
            }
        }
    }
    return result;
}

/**
 * Computes the scalar product of the given matrix and the given value.
 *
 * @tparam T the component type
 * @tparam R the number of rows
 * @tparam C the number of columns
 * @param lhs the matrix
 * @param rhs the scalar
 * @return the product of the given matrix and the given value
 */
template <typename T, size_t R, size_t C>
mat<T,R,C> operator*(const mat<T,R,C>& lhs, const T rhs) {
    mat<T,R,C> result;
    for (size_t c = 0; c < C; ++c) {
        result[c] = lhs[c] * rhs;
    }
    return result;
}

/**
 * Computes the scalar product of the given matrix and the given value.
 *
 * @tparam T the component type
 * @tparam R the number of rows
 * @tparam C the number of columns
 * @param lhs the scalar
 * @param rhs the matrix
 * @return the product of the given matrix and the given value
 */
template <typename T, size_t R, size_t C>
mat<T,R,C> operator*(const T lhs, const mat<T,R,C>& rhs) {
    return rhs * lhs;
}

/**
 * Computes the scalar division of the given matrix and the given value.
 *
 * @tparam T the component type
 * @tparam R the number of rows
 * @tparam C the number of columns
 * @param lhs the matrix
 * @param rhs the scalar
 * @return the division of the given matrix and the given value
 */
template <typename T, size_t R, size_t C>
mat<T,R,C> operator/(const mat<T,R,C>& lhs, const T rhs) {
    mat<T,R,C> result;
    for (size_t c = 0; c < C; ++c) {
        result[c] = lhs[c] / rhs;
    }
    return result;
}

/**
 * Multiplies the given vector by the given matrix.
 *
 * @tparam T the component type
 * @tparam R the number of rows
 * @tparam C the number of columns
 * @param lhs the vector
 * @param rhs the matrix
 * @return the product of the given vector and the given matrix
 */
template <typename T, size_t R, size_t C>
const vec<T,R> operator*(const vec<T,R>& lhs, const mat<T,R,C>& rhs) {
    vec<T,R> result;
    for (size_t c = 0; c < C; c++) {
        result[c] = dot(lhs, rhs[c]);
    }
    return result;
}

/**
 * Multiplies the given vector by the given matrix.
 *
 * @tparam T the component type
 * @tparam R the number of rows
 * @tparam C the number of columns
 * @param lhs the matrix
 * @param rhs the vector
 * @return the product of the given vector and the given matrix
 */
template <typename T, size_t R, size_t C>
vec<T,R> operator*(const mat<T,R,C>& lhs, const vec<T,C>& rhs) {
    vec<T,C> result;
    for (size_t r = 0; r < R; r++) {
        for (size_t c = 0; c < C; ++c) {
            result[r] += lhs[c][r] * rhs[c];
        }
    }
    return result;
}

/**
 * Multiplies the given vector by the given matrix.
 *
 * @tparam T the component type
 * @tparam R the number of rows
 * @tparam C the number of columns
 * @param lhs the matrix
 * @param rhs the vector
 * @return the product of the given vector and the given matrix
 */
template <typename T, size_t R, size_t C>
vec<T,C-1> operator*(const mat<T,R,C>& lhs, const vec<T,C-1>& rhs) {
    return toCartesianCoords(lhs * toHomogeneousCoords(rhs));
}

/**
 * Multiplies the given vector by the given matrix.
 *
 * @tparam T the component type
 * @tparam R the number of rows
 * @tparam C the number of columns
 * @param lhs the vector
 * @param rhs the matrix
 * @return the product of the given vector and the given matrix
 */
template <typename T, size_t R, size_t C>
const vec<T,R-1> operator*(const vec<T,R-1>& lhs, const mat<T,R,C>& rhs) {
    return toCartesianCoords(toHomogeneousCoords(lhs) * rhs);
}

/**
 * Multiplies the given list of vectors with the given matrix.
 *
 * @tparam T the component type
 * @tparam R the number of rows
 * @tparam C the number of columns
 * @param lhs the list of vectors
 * @param rhs the matrix
 * @return a list of the the products of the given vectors and the given matrix
 */
template <typename T, size_t R, size_t C>
typename vec<T,R>::List operator*(const typename vec<T,R>::List& lhs, const mat<T,R,C>& rhs) {
    typename vec<T,R>::List result;
    result.reserve(lhs.size());
    std::transform(std::begin(lhs), std::end(lhs), std::back_inserter(result), [rhs](const vec<T,R>& elem) { return elem * rhs; });
    return result;
}

/**
 * Multiplies the given list of vectors with the given matrix.
 *
 * @tparam T the component type
 * @tparam R the number of rows
 * @tparam C the number of columns
 * @param lhs the list of vectors
 * @param rhs the matrix
 * @return a list of the the products of the given vectors and the given matrix
 */
template <typename T, size_t R, size_t C>
typename vec<T,R-1>::List operator*(const typename vec<T,R-1>::List& lhs, const mat<T,R,C>& rhs) {
    typename vec<T,R-1>::List result;
    result.reserve(lhs.size());
    std::transform(std::begin(lhs), std::end(lhs), std::back_inserter(result), [rhs](const vec<T,R-1>& elem) { return elem * rhs; });
    return result;
}

/**
 * Multiplies the given list of vectors with the given matrix.
 *
 * @tparam T the component type
 * @tparam R the number of rows
 * @tparam C the number of columns
 * @param lhs the list of vectors
 * @param rhs the matrix
 * @return a list of the the products of the given vectors and the given matrix
 */
template <typename T, size_t R, size_t C>
typename vec<T,C>::List operator*(const mat<T,R,C>& lhs, const typename vec<T,C>::List& rhs) {
    typename vec<T,C>::List result;
    result.reserve(rhs.size());
    std::transform(std::begin(rhs), std::end(rhs), std::back_inserter(result), [lhs](const vec<T,C>& elem) { return lhs * elem; });
    return result;
}

/**
 * Multiplies the given list of vectors with the given matrix.
 *
 * @tparam T the component type
 * @tparam R the number of rows
 * @tparam C the number of columns
 * @param lhs the list of vectors
 * @param rhs the matrix
 * @return a list of the the products of the given vectors and the given matrix
 */
template <typename T, size_t R, size_t C>
typename vec<T,C-1>::List operator*(const mat<T,R,C>& lhs, const typename vec<T,C-1>::List& rhs) {
    typename vec<T,C-1>::List result;
    result.reserve(rhs.size());
    std::transform(std::begin(rhs), std::end(rhs), std::back_inserter(result), [lhs](const vec<T,C-1>& elem) { return lhs * elem; });
    return result;
}

/**
 * Transposes the given square matrix.
 *
 * @tparam T the component type
 * @tparam S the number of rows and columns
 * @param m the matrix to transpose
 * @return the transposed matrix
 */
template <typename T, size_t S>
mat<T,S,S> transpose(const mat<T,S,S>& m) {
    using std::swap;
    mat<T,S,S> result(m);
    for (size_t c = 0; c < S; c++) {
        for (size_t r = c + 1; r < S; r++) {
            swap(result[c][r], result[r][c]);
        }
    }
    return result;
}

/**
 * Computes a minor of the given square matrix. The minor of a matrix is obtained by erasing one column
 * and one row from that matrix. Thus, any minor matrix of an n*n matrix is a (n-1)*(n-1) matrix.
 *
 * @tparam T the component type
 * @tparam S the number of rows and columns of the given matrix
 * @param m the matrix to compute a minor of
 * @param row the row to strike
 * @param col the column to strike
 * @return the minor matrix
 */
template <typename T, size_t S>
const mat<T,S-1,S-1> extractMinor(const mat<T,S,S>& m, const size_t row, const size_t col) {
    mat<T,S-1,S-1> min;
    size_t minC, minR;
    minC = 0;
    for (size_t c = 0; c < S; c++) {
        if (c != col) {
            minR = 0;
            for (size_t r = 0; r < S; r++) {
                if (r != row) {
                    min[minC][minR++] = m[c][r];
                }
            }
            minC++;
        }
    }
    return min;
}

/**
 * Helper template to compute the determinant of any square matrix using Laplace expansion
 * after the first column.
 *
 * @see https://en.wikipedia.org/wiki/Laplace_expansion
 *
 * @tparam T the component type
 * @tparam S the number of components
 */
template <typename T, size_t S>
struct MatrixDeterminant {
    T operator() (const mat<T,S,S>& m) const {
        // Laplace after first col
        auto det = static_cast<T>(0.0);
        for (size_t r = 0; r < S; r++) {
            const auto f = static_cast<T>(r % 2 == 0 ? 1.0 : -1.0);
            det += f * m[0][r] * MatrixDeterminant<T,S-1>()(extractMinor(m, r, 0));
        }
        return det;
    }
};

// TODO: implement faster block-matrix based method for NxN matrices where N = 2^n

/**
 * Partial specialization helper template to compute the determinant of a 3*3 matrix using the
 * rule of Sarrus.
 *
 * @see https://en.wikipedia.org/wiki/Rule_of_Sarrus
 *
 * @tparam T the component type
 */
template <typename T>
struct MatrixDeterminant<T,3> {
    T operator() (const mat<T,3,3>& m) const {
        return (  m[0][0]*m[1][1]*m[2][2]
                  + m[1][0]*m[2][1]*m[0][2]
                  + m[2][0]*m[0][1]*m[1][2]
                  - m[2][0]*m[1][1]*m[0][2]
                  - m[1][0]*m[0][1]*m[2][2]
                  - m[0][0]*m[2][1]*m[1][2]);
    }
};

/**
 * Partial specialization helper template to compute the determinant of a 2*2 matrix using the
 * rule of Sarrus.
 *
 * @see https://en.wikipedia.org/wiki/Rule_of_Sarrus
 *
 * @tparam T the component type
 */
template <typename T>
struct MatrixDeterminant<T,2> {
    T operator() (const mat<T,2,2>& m) const {
        return (  m[0][0]*m[1][1]
                  - m[1][0]*m[0][1]);
    }
};

/**
 * Partial specialization helper template to compute the determinant of a 1*1 matrix.
 *
 * @tparam T the component type
 */
template <typename T>
struct MatrixDeterminant<T,1> {
    T operator() (const mat<T,1,1>& m) const {
        return m[0][0];
    }
};

/**
 * Computes the determinant of the given square matrix.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param m the matrix to compute the determinant of
 * @return the determinant of the given matrix
 */
template <typename T, size_t S>
T computeDeterminant(const mat<T,S,S>& m) {
    return MatrixDeterminant<T,S>()(m);
}

/**
 * Computes the adjugate of the given square matrix.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param m the matrix to compute the adjugate of
 * @return the adjugate of the given matrix
 */
template <typename T, size_t S>
mat<T,S,S> computeAdjugate(const mat<T,S,S>& m) {
    mat<T,S,S> result;
    for (size_t c = 0; c < S; c++) {
        for (size_t r = 0; r < S; r++) {
            const auto f = static_cast<T>((c + r) % 2 == 0 ? 1.0 : -1.0);
            result[r][c] = f * computeDeterminant(extractMinor(m, r, c)); // transpose the matrix on the fly
        }
    }
    return result;
}

/**
 * Inverts the given square matrix if possible.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param m the matrix to invert
 * @return a pair of a boolean and a matrix such that the boolean indicates whether the
 * matrix is invertible, and if so, the matrix is the inverted given matrix
 */
template <typename T, size_t S>
std::tuple<bool, mat<T,S,S>> invert(const mat<T,S,S>& m) {
    const auto det = computeDeterminant(m);
    const auto invertible = (det != static_cast<T>(0.0));
    if (!invertible) {
        return std::make_tuple(false, mat<T,S,S>::identity);
    } else {
        return std::make_tuple(true, computeAdjugate(m) / det);
    }
}

/**
 * Returns a perspective camera transformation with the given parameters. The returned matrix transforms from eye
 * coordinates to clip coordinates.
 *
 * @tparam T the component type
 * @param fov the field of view, in degrees
 * @param nearPlane the distance to the near plane
 * @param farPlane the distance to the far plane
 * @param width the viewport width
 * @param height the viewport height
 * @return the perspective transformation matrix
 */
template <typename T>
mat<T,4,4> perspectiveMatrix(const T fov, const T nearPlane, const T farPlane, const int width, const int height) {
    const auto vFrustum = std::tan(Math::radians(fov) / static_cast<T>(2.0)) * static_cast<T>(0.75) * nearPlane;
    const auto hFrustum = vFrustum * static_cast<T>(width) / static_cast<T>(height);
    const auto depth = farPlane - nearPlane;

    static const auto zero = static_cast<T>(0.0);
    static const auto one  = static_cast<T>(1.0);
    static const auto two  = static_cast<T>(2.0);

    return mat<T,4,4>(nearPlane / hFrustum, zero,                    zero,                               zero,
                      zero,                 nearPlane / vFrustum,    zero,                               zero,
                      zero,                 zero,                   -(farPlane + nearPlane) / depth,    -two * farPlane * nearPlane / depth,
                      zero,                 zero,                   -one,                                zero);
}

/**
 * Returns an orthographic camera transformation with the given parameters. The origin of the given screen coordinates
 * is at the center. The returned matrix transforms from eye coordinates to clip coordinates.
 *
 * @tparam T the component type
 * @param nearPlane the distance to the near plane
 * @param farPlane the distance to the far plane
 * @param left the screen coordinate of the left border of the viewport
 * @param top the screen coordinate of the top border of the viewport
 * @param right the screen coordinate of the right border of the viewport
 * @param bottom the screen coordinate of the bottom border of the viewport
 * @return the orthographic transformation matrix
 */
template <typename T>
mat<T,4,4> orthoMatrix(const T nearPlane, const T farPlane, const T left, const T top, const T right, const T bottom) {
    const auto width = right - left;
    const auto height = top - bottom;
    const auto depth = farPlane - nearPlane;

    static const auto zero = static_cast<T>(0.0);
    static const auto one  = static_cast<T>(1.0);
    static const auto two  = static_cast<T>(2.0);

    return mat<T,4,4>(two / width,  zero,            zero,          -(left + right) / width,
                      zero,         two / height,    zero,          -(top + bottom) / height,
                      zero,         zero,           -two / depth,   -(farPlane + nearPlane) / depth,
                      zero,         zero,            zero,           one);
}

/**
 * Returns a view transformation matrix which transforms normalized device coordinates to window coordinates.
 *
 * @tparam T the component type
 * @param direction the view direction
 * @param up the up vector
 * @return the view transformation matrix
 */
template <typename T>
mat<T,4,4> viewMatrix(const vec<T,3>& direction, const vec<T,3>& up) {
    const auto& f = direction;
    const auto  s = cross(f, up);
    const auto  u = cross(s, f);

    static const auto zero = static_cast<T>(0.0);
    static const auto one  = static_cast<T>(1.0);

    return mat<T,4,4>( s[0],  s[1],  s[2], zero,
                       u[0],  u[1],  u[2], zero,
                       -f[0], -f[1], -f[2], zero,
                       zero,  zero,  zero, one);
}

/**
 * Returns a matrix that will rotate a point counter clockwise by the given angles. The rotation is applied in the same
 * order the parameters are given: first roll, then pitch, then yaw.
 *
 * @tparam T the component type
 * @param roll the roll angle (in radians)
 * @param pitch the pitch angle (in radians)
 * @param yaw the yaw angle (in radians)
 * @return the rotation matrix
 */
template <typename T>
mat<T,4,4> rotationMatrix(const T roll, const T pitch, const T yaw) {
    static const auto I = static_cast<T>(1.0);
    static const auto O = static_cast<T>(0.0);

    const auto Cr = std::cos(roll);
    const auto Sr = std::sin(roll);
    const  mat<T,4,4> R( +I,  +O,  +O,  +O,
                         +O, +Cr, -Sr,  +O,
                         +O, +Sr, +Cr,  +O,
                         +O,  +O,  +O,  +I);

    const auto Cp = std::cos(pitch);
    const auto Sp = std::sin(pitch);
    const mat<T,4,4> P(+Cp,  +O, +Sp,  +O,
                       +O,  +I,  +O,  +O,
                       -Sp,  +O, +Cp,  +O,
                       +O,  +O,  +O,  +I);

    const auto Cy = std::cos(yaw);
    const auto Sy = std::sin(yaw);
    const mat<T,4,4> Y(+Cy, -Sy,  +O,  +O,
                       +Sy, +Cy,  +O,  +O,
                       +O,  +O,  +I,  +O,
                       +O,  +O,  +O,  +I);

    return Y * P * R;
}

/**
 * Returns a matrix that will rotate a point counter clockwise about the given axis by the given angle.
 *
 * @tparam T the component type
 * @param axis the axis to rotate about
 * @param angle the rotation angle (in radians)
 * @return the rotation matrix
 */
template <typename T>
mat<T,4,4> rotationMatrix(const vec<T,3>& axis, const T angle) {
    const auto s = std::sin(-angle);
    const auto c = std::cos(-angle);
    const auto i = static_cast<T>(1.0 - c);

    const auto ix  = i  * axis[0];
    const auto ix2 = ix * axis[0];
    const auto ixy = ix * axis[1];
    const auto ixz = ix * axis[2];

    const auto iy  = i  * axis[1];
    const auto iy2 = iy * axis[1];
    const auto iyz = iy * axis[2];

    const auto iz2 = i  * axis[2] * axis[2];

    const auto sx = s * axis[0];
    const auto sy = s * axis[1];
    const auto sz = s * axis[2];

    mat<T,4,4> rotation;
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

/**
 * Returns a rotation matrix that performs the same rotation as the given quaternion.
 *
 * @see http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToMatrix/
 *
 * @tparam T the component type
 * @param quat the quaternion
 * @return the rotation matrix
 */
template <typename T>
mat<T,4,4> rotationMatrix(const Quat<T>& quat) {
    const auto x = quat.v[0];
    const auto y = quat.v[1];
    const auto z = quat.v[2];
    const auto w = quat.r;

    const auto x2 = x*x;
    const auto y2 = y*y;
    const auto z2 = z*z;

    mat<T,4,4> rotation;
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
 * Returns a matrix that will rotate the first given vector onto the second given vector about their perpendicular
 * axis. The vectors are expected to be normalized.
 *
 * @tparam T the component type
 * @param from the vector to rotate
 * @param to the vector to rotate onto
 * @return the rotation matrix
 */
template <typename T>
mat<T,4,4> rotationMatrix(const vec<T,3>& from, const vec<T,3>& to) {
    return rotationMatrix(Quat<T>(from, to));
}

/**
 * Returns a matrix that translates by the given delta.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param delta the deltas by which to translate
 * @return the translation matrix
 */
template <typename T, size_t S>
mat<T,S+1,S+1> translationMatrix(const vec<T,S>& delta) {
    mat<T,S+1,S+1> translation;
    for (size_t i = 0; i < S; ++i) {
        translation[S][i] = delta[i];
    }
    return translation;
}

/**
 * Returns a matrix that contains only the translation part of the given transformation matrix.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param m the transformation matrix
 * @return the translation matrix
 */
template <typename T, size_t S>
mat<T,S,S> translationMatrix(const mat<T,S,S>& m) {
    mat<T,S,S> result;
    for (size_t i = 0; i < S-1; ++i) {
        result[S-1][i] = m[S-1][i];
    }
    return result;
}

/**
 * Strips the translation part from the given transformation matrix.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param m the transformation matrix
 * @return the transformation matrix without its translation part
 */
template <typename T, size_t S>
mat<T,S,S> stripTranslation(const mat<T,S,S>& m) {
    mat<T,S,S> result(m);
    for (size_t i = 0; i < S-1; ++i) {
        result[S-1][i] = static_cast<T>(0.0);
    }
    return result;
}

/**
 * Returns a scaling matrix with the given scaling factors.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param factors the scaling factors
 * @return the scaling matrix
 */
template <typename T, size_t S>
mat<T,S+1,S+1> scalingMatrix(const vec<T,S>& factors) {
    mat<T,S+1,S+1> scaling;
    for (size_t i = 0; i < S; ++i) {
        scaling[i][i] = factors[i];
    }
    return scaling;
}

/**
 * Returns a matrix that mirrors along the given axis.
 *
 * @tparam T the component type
 * @param axis the axis along which to mirror
 * @return the mirroring axis
 */
template <typename T>
const mat<T,4,4>& mirrorMatrix(const Math::Axis::Type axis) {
    switch (axis) {
        case Math::Axis::AX:
            return mat<T,4,4>::mirror_x;
        case Math::Axis::AY:
            return mat<T,4,4>::mirror_y;
        case Math::Axis::AZ:
            return mat<T,4,4>::mirror_z;
        default:
            return mat<T,4,4>::identity;
    }
}

/**
 * Returns a matrix that transforms to a coordinate system specified by the given axes and offset.
 *
 * @tparam T the component type
 * @param x the X axis of the target coordinate system, expressed relative to the source coordinate system
 * @param y the Y axis of the target coordinate system, expressed relative to the source coordinate system
 * @param z the Z axis of the target coordinate system, expressed relative to the source coordinate system
 * @param o the offset of the target coordinate system, expressed relative to the source coordinate system
 * @return the transformation matrix
 */
template <typename T>
mat<T,4,4> coordinateSystemMatrix(const vec<T,3>& x, const vec<T,3>& y, const vec<T,3>& z, const vec<T,3>& o) {
    const auto [invertible, result] = invert(mat<T,4,4>(x[0], y[0], z[0], o[0],
                                                        x[1], y[1], z[1], o[1],
                                                        x[2], y[2], z[2], o[2],
                                                        0.0,  0.0,  0.0,  1.0));
    assert(invertible);
    return result;
}

/**
 * Returns a matrix that will transform a point to a coordinate system where the X and
 * Y axes are in the given plane and the Z axis is parallel to the given direction. This is useful for
 * projecting points onto a plane along a particular direction.
 *
 * @tparam T the component type
 * @param distance the distance of the plane
 * @param normal the normal of the plane
 * @param direction the projection direction
 * @return the transformation matrix
 */
template <typename T>
mat<T,4,4> planeProjectionMatrix(const T distance, const vec<T,3>& normal, const vec<T,3>& direction) {
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
    const auto  yAxis = normalize(cross(normal, xAxis));
    const auto& zAxis = direction;

    assert(Math::eq(length(xAxis), 1.0));
    assert(Math::eq(length(yAxis), 1.0));
    assert(Math::eq(length(zAxis), 1.0));

    return coordinateSystemMatrix(xAxis, yAxis, zAxis, distance * normal);
}

/**
 Returns the inverse of a matrix that will transform a point to a coordinate system where the X and
 Y axes are in the given plane and the Z axis is the given normal.
 */

/**
 * Returns a matrix that will transform a point to a coordinate system where the X and
 * Y axes are in the given plane and the Z axis is the plane normal. This is useful for vertically
 * projecting points onto a plane.
 *
 * @tparam T the component type
 * @param distance the distance of the plane
 * @param normal the normal of the plane
 * @return the transformation matrix
 */
template <typename T>
mat<T,4,4> planeProjectionMatrix(const T distance, const vec<T,3>& normal) {
    return planeProjectionMatrix(distance, normal, normal);
}

/**
From: http://web.archive.org:80/web/20041029003853/http://www.j3d.org/matrix_faq/matrfaq_latest.html#Q43
*/

/**
 * Returns a matrix that performs a shearing transformation. In 3D, six shearing directions are possible:
 *
 * - X in direction of Y
 * - X in direction of Z
 * - Y in direction of X
 * - Y in direction of Z
 * - Z in direction of X
 * - Z in direction of Y
 *
 * @tparam T the component type
 * @param Sxy amount by which to share the X axis in direction of the Y axis
 * @param Sxz amount by which to share the X axis in direction of the Z axis
 * @param Syx amount by which to share the Y axis in direction of the X axis
 * @param Syz amount by which to share the Y axis in direction of the Z axis
 * @param Szx amount by which to share the Z axis in direction of the X axis
 * @param Szy amount by which to share the Z axis in direction of the Y axis
 * @return the shearing matrix
 */
template <typename T>
mat<T,4,4> shearMatrix(const T Sxy, const T Sxz, const T Syx, const T Syz, const T Szx, const T Szy) {
    return mat<T,4,4>(1.0, Syx, Szx, 0.0,
                      Sxy, 1.0, Szy, 0.0,
                      Sxz, Syz, 1.0, 0.0,
                      0.0, 0.0, 0.0, 1.0);
}

#endif //TRENCHBROOM_MAT_FUNCTIONS_H
