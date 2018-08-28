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
    using Type = T;
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

    using List = std::vector<Mat<T,R,C>>;
    
    /**
     * The matrix components in column major format.
     */
    vec<T,R> v[C];
public:
    /**
     * Returns a matrix where all components are set to the given value.
     *
     * @param value the value to set
     * @return the newly created matrix
     */
    static Mat<T,R,C> fill(const T value) {
        Mat<T,R,C> result;
        for (size_t c = 0; c < C; c++) {
            result[c] = vec<T,R>::fill(value);
        }
        return result;
    }

    /**
     * Returns an identity matrix.
     *
     * @return a matrix with all values of the diagonal set to 1 and all other values set to 0
     */
    static Mat<T,R,C> identity() {
        Mat<T,R,C> result;
        for (size_t c = 0; c < C; c++) {
            for (size_t r = 0; r < R; r++) {
                if (c == r) {
                    result[c][r] = static_cast<T>(1.0);
                }
            }
        }
        return result;
    }
public:
    Mat<T,R,C>() {
        for (size_t c = 0; c < C; c++) {
            for (size_t r = 0; r < R; r++) {
                v[c][r] = c == r ? static_cast<T>(1.0) : static_cast<T>(0.0);
            }
        }
    }
    
    // Copy and move constructors
    Mat(const Mat<T,R,C>& other) = default;
    Mat(Mat<T,R,C>&& other) = default;
    
    // Assignment operators
    Mat<T,R,C>& operator=(const Mat<T,R,C>& other) = default;
    Mat<T,R,C>& operator=(Mat<T,R,C>&& other) = default;

    /**
     * Sets the values of the newly created matrix to the values of the given matrix and casts each value of the given
     * matrix to the component type of the newly created matrix.
     *
     * @tparam U the component type of the source matrix
     * @param other the source matrix
     */
    template <typename U>
    Mat(const Mat<U,R,C>& other) {
        for (size_t c = 0; c < C; ++c) {
            for (size_t r = 0; r < R; ++r) {
                v[c][r] = static_cast<T>(other[c][r]);
            }
        }
    }

    /**
     * Sets the values of the newly created matrix to the given values, and all other values to 0.
     *
     * @param v11 the value at column 1 and row 1
     * @param v12 the value at column 2 and row 1
     * @param v13 the value at column 3 and row 1
     * @param v21 the value at column 1 and row 2
     * @param v22 the value at column 2 and row 2
     * @param v23 the value at column 3 and row 2
     * @param v31 the value at column 1 and row 3
     * @param v32 the value at column 2 and row 3
     * @param v33 the value at column 3 and row 3
     */
    Mat<T,R,C>(const T v11, const T v12, const T v13,
               const T v21, const T v22, const T v23,
               const T v31, const T v32, const T v33) {
        v[0][0] = v11; v[1][0] = v12; v[2][0] = v13;
        v[0][1] = v21; v[1][1] = v22; v[2][1] = v23;
        v[0][2] = v31; v[1][2] = v32; v[2][2] = v33;
        for (size_t c = 3; c < C; c++) {
            for (size_t r = 3; r < R; r++) {
                v[c][r] = static_cast<T>(0.0);
            }
        }
    }

    /**
     * Sets the values of the newly created matrix to the given values, and all other values to 0.
     *
     * @param v11 the value at column 1 and row 1
     * @param v12 the value at column 2 and row 1
     * @param v13 the value at column 3 and row 1
     * @param v14 the value at column 4 and row 1
     * @param v21 the value at column 1 and row 2
     * @param v22 the value at column 2 and row 2
     * @param v23 the value at column 3 and row 2
     * @param v24 the value at column 4 and row 2
     * @param v31 the value at column 1 and row 3
     * @param v32 the value at column 2 and row 3
     * @param v33 the value at column 3 and row 3
     * @param v34 the value at column 4 and row 3
     * @param v41 the value at column 1 and row 4
     * @param v42 the value at column 2 and row 4
     * @param v43 the value at column 3 and row 4
     * @param v44 the value at column 4 and row 4
     */
    Mat<T,R,C>(const T v11, const T v12, const T v13, const T v14,
               const T v21, const T v22, const T v23, const T v24,
               const T v31, const T v32, const T v33, const T v34,
               const T v41, const T v42, const T v43, const T v44) {
        v[0][0] = v11; v[1][0] = v12; v[2][0] = v13; v[3][0] = v14;
        v[0][1] = v21; v[1][1] = v22; v[2][1] = v23; v[3][1] = v24;
        v[0][2] = v31; v[1][2] = v32; v[2][2] = v33; v[3][2] = v34;
        v[0][3] = v41; v[1][3] = v42; v[2][3] = v43; v[3][3] = v44;
        for (size_t c = 4; c < C; c++) {
            for (size_t r = 4; r < R; r++) {
                v[c][r] = static_cast<T>(0.0);
            }
        }
    }

    /**
     * Returns the column at the given index.
     *
     * @param index the index of the column to return
     * @return the column at the given index
     */
    vec<T,R>& operator[] (const size_t index) {
        assert(index < C);
        return v[index];
    }

    /**
     * Returns the column at the given index.
     *
     * @param index the index of the column to return
     * @return the column at the given index
     */
    const vec<T,R>& operator[] (const size_t index) const {
        assert(index < C);
        return v[index];
    }
};

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
int compare(const Mat<T,R,C>& lhs, const Mat<T,R,C>& rhs, const T epsilon) {
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
bool operator==(const Mat<T,R,C>& lhs, const Mat<T,R,C>& rhs) {
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
bool operator!=(const Mat<T,R,C>& lhs, const Mat<T,R,C>& rhs) {
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
bool equal(const Mat<T,R,C>& lhs, const Mat<T,R,C>& rhs, const T epsilon) {
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
bool isZero(const Mat<T,R,C>& m, const T epsilon = Math::Constants<T>::almostZero()) {
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
Mat<T,R,C> operator-(const Mat<T,R,C>& m) {
    Mat<T,R,C> result;
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
Mat<T,R,C> operator+(const Mat<T,R,C>& lhs, const Mat<T,R,C>& rhs) {
    Mat<T,R,C> result;
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
Mat<T,R,C> operator-(const Mat<T,R,C>& lhs, const Mat<T,R,C>& rhs) {
    Mat<T,R,C> result;
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
Mat<T,R,C> operator*(const Mat<T,C,R>& lhs, const Mat<T,C,R>& rhs) {
    auto result = Mat<T,R,C>::Null;
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
Mat<T,R,C> operator*(const Mat<T,R,C>& lhs, const T rhs) {
    Mat<T,R,C> result;
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
Mat<T,R,C> operator*(const T lhs, const Mat<T,R,C>& rhs) {
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
Mat<T,R,C> operator/(const Mat<T,R,C>& lhs, const T rhs) {
    Mat<T,R,C> result;
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
const vec<T,R> operator*(const vec<T,R>& lhs, const Mat<T,R,C>& rhs) {
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
vec<T,R> operator*(const Mat<T,R,C>& lhs, const vec<T,C>& rhs) {
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
vec<T,C-1> operator*(const Mat<T,R,C>& lhs, const vec<T,C-1>& rhs) {
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
const vec<T,R-1> operator*(const vec<T,R-1>& lhs, const Mat<T,R,C>& rhs) {
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
typename vec<T,R>::List operator*(const typename vec<T,R>::List& lhs, const Mat<T,R,C>& rhs) {
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
typename vec<T,R-1>::List operator*(const typename vec<T,R-1>::List& lhs, const Mat<T,R,C>& rhs) {
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
typename vec<T,C>::List operator*(const Mat<T,R,C>& lhs, const typename vec<T,C>::List& rhs) {
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
typename vec<T,C-1>::List operator*(const Mat<T,R,C>& lhs, const typename vec<T,C-1>::List& rhs) {
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
 * @param mat the matrix to transpose
 * @return the transposed matrix
 */
template <typename T, size_t S>
Mat<T,S,S> transpose(const Mat<T,S,S>& mat) {
    using std::swap;
    Mat<T,S,S> result(mat);
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
const Mat<T,S-1,S-1> minor(const Mat<T,S,S>& m, const size_t row, const size_t col) {
    Mat<T,S-1,S-1> min;
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
    T operator() (const Mat<T,S,S>& m) const {
        // Laplace after first col
        T det = static_cast<T>(0.0);
        for (size_t r = 0; r < S; r++) {
            const T f = static_cast<T>(r % 2 == 0 ? 1.0 : -1.0);
            det += f * m[0][r] * MatrixDeterminant<T,S-1>()(minor(m, r, 0));
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
    T operator() (const Mat<T,3,3>& m) const {
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
    T operator() (const Mat<T,2,2>& m) const {
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
    T operator() (const Mat<T,1,1>& m) const {
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
T determinant(const Mat<T,S,S>& m) {
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
Mat<T,S,S> adjugate(const Mat<T,S,S>& m) {
    Mat<T,S,S> result;
    for (size_t c = 0; c < S; c++) {
        for (size_t r = 0; r < S; r++) {
            const T f = static_cast<T>((c + r) % 2 == 0 ? 1.0 : -1.0);
            result[r][c] = f * determinant(minor(m, r, c)); // transpose the matrix on the fly
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
std::tuple<bool, Mat<T,S,S>> invert(const Mat<T,S,S>& m) {
    const auto det = determinant(m);
    const auto invertible = (det != static_cast<T>(0.0));
    if (!invertible) {
        return std::make_tuple(false, Mat<T,S,S>::Identity);
    } else {
        return std::make_tuple(true, adjugate(m) / det);
    }
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
Mat<T,S,S> translationMatrix(const Mat<T,S,S>& m) {
    Mat<T,S,S> result;
    for (size_t i = 0; i < S-1; ++i)
        result[S-1][i] = m[S-1][i];
    return result;
}

template <typename T, size_t S>
Mat<T,S,S> stripTranslation(const Mat<T,S,S>& m) {
    Mat<T,S,S> result(m);
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
    const auto [invertible, result] = invert(Mat<T,4,4>(x[0], y[0], z[0], o[0],
                                                        x[1], y[1], z[1], o[1],
                                                        x[2], y[2], z[2], o[2],
                                                        0.0,  0.0,  0.0,  1.0));
    assert(invertible); unused(invertible);
    return result;
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
const Mat<T,R,C> Mat<T,R,C>::Identity = Mat<T,R,C>::identity();

template <typename T, size_t R, size_t C>
const Mat<T,R,C> Mat<T,R,C>::Null = Mat<T,R,C>::fill(static_cast<T>(0.0));

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
