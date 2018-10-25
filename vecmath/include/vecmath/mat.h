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

#ifndef TRENCHBROOM_MAT_DECL_H
#define TRENCHBROOM_MAT_DECL_H

#include "vec.h"
#include "quat.h"
#include "constants.h"
#include "util.h"

#include <cassert>
#include <tuple>

namespace vm {
    template <typename T, size_t R, size_t C>
    class mat {
    public:
        using Type = T;
        static const size_t Rows = R;
        static const size_t Cols = C;

        static const mat<T,R,C> zero;
        static const mat<T,R,C> identity;
        static const mat<T,R,C> rot_90_x_cw;
        static const mat<T,R,C> rot_90_y_cw;
        static const mat<T,R,C> rot_90_z_cw;
        static const mat<T,R,C> rot_90_x_ccw;
        static const mat<T,R,C> rot_90_y_ccw;
        static const mat<T,R,C> rot_90_z_ccw;
        static const mat<T,R,C> rot_180_x;
        static const mat<T,R,C> rot_180_y;
        static const mat<T,R,C> rot_180_z;
        static const mat<T,R,C> mirror_x;
        static const mat<T,R,C> mirror_y;
        static const mat<T,R,C> mirror_z;
        static const mat<T,R,C> zero_x;
        static const mat<T,R,C> zero_y;
        static const mat<T,R,C> zero_z;
        static const mat<T,R,C> yiq_to_rgb;
        static const mat<T,R,C> rgb_to_yiq;

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
        static mat<T,R,C> fill(const T value) {
            mat<T,R,C> result;
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
        static mat<T,R,C> identityMatrix() {
            mat<T,R,C> result;
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
        /**
         * Creates a new matrix with its values initialized to an identity matrix.
         */
        mat() {
            for (size_t c = 0; c < C; c++) {
                for (size_t r = 0; r < R; r++) {
                    v[c][r] = c == r ? static_cast<T>(1.0) : static_cast<T>(0.0);
                }
            }
        }

        // Copy and move constructors
        mat(const mat<T,R,C>& other) = default;
        mat(mat<T,R,C>&& other) noexcept = default;

        // Assignment operators
        mat<T,R,C>& operator=(const mat<T,R,C>& other) = default;
        mat<T,R,C>& operator=(mat<T,R,C>&& other) noexcept = default;

        /**
         * Sets the values of the newly created matrix to the values of the given matrix and casts each value of the given
         * matrix to the component type of the newly created matrix.
         *
         * @tparam U the component type of the source matrix
         * @param other the source matrix
         */
        template <typename U>
        explicit mat(const mat<U,R,C>& other) {
            for (size_t c = 0; c < C; ++c) {
                for (size_t r = 0; r < R; ++r) {
                    v[c][r] = static_cast<T>(other[c][r]);
                }
            }
        }

        /**
         * Sets the values of the newly created matrix to the given values, which are given row-by-row.
         *
         * @param list the initializer list with the matrix elements in row-major order
         */
        mat(std::initializer_list<T> list) {
            assert(list.size() == R*C);
            const T* listPtr = list.begin();

            for (size_t c = 0; c < C; ++c) {
                for (size_t r = 0; r < R; ++r) {
                    v[c][r] = listPtr[c + (C * r)];
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
        mat(const T v11, const T v12, const T v13,
            const T v21, const T v22, const T v23,
            const T v31, const T v32, const T v33) {
            static_assert(C == 3 && R == 3, "constructor only available for 3x3 matrices");
            v[0][0] = v11; v[1][0] = v12; v[2][0] = v13;
            v[0][1] = v21; v[1][1] = v22; v[2][1] = v23;
            v[0][2] = v31; v[1][2] = v32; v[2][2] = v33;
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
        mat(const T v11, const T v12, const T v13, const T v14,
            const T v21, const T v22, const T v23, const T v24,
            const T v31, const T v32, const T v33, const T v34,
            const T v41, const T v42, const T v43, const T v44) {
            static_assert(C == 4 && R == 4, "constructor only available for 4x4 matrices");
            v[0][0] = v11; v[1][0] = v12; v[2][0] = v13; v[3][0] = v14;
            v[0][1] = v21; v[1][1] = v22; v[2][1] = v23; v[3][1] = v24;
            v[0][2] = v31; v[1][2] = v32; v[2][2] = v33; v[3][2] = v34;
            v[0][3] = v41; v[1][3] = v42; v[2][3] = v43; v[3][3] = v44;
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

    template <typename T, size_t R, size_t C>
    const mat<T,R,C> mat<T,R,C>::identity = mat<T,R,C>::identityMatrix();

    template <typename T, size_t R, size_t C>
    const mat<T,R,C> mat<T,R,C>::zero = mat<T,R,C>::fill(static_cast<T>(0.0));

    template <typename T, size_t R, size_t C>
    const mat<T,R,C> mat<T,R,C>::rot_90_x_cw  = mat<T,R,C>(+static_cast<T>(1.0), +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(0.0),
                                                           +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(1.0), +static_cast<T>(0.0),
                                                           +static_cast<T>(0.0), -static_cast<T>(1.0), +static_cast<T>(0.0), +static_cast<T>(0.0),
                                                           +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(1.0));
    template <typename T, size_t R, size_t C>
    const mat<T,R,C> mat<T,R,C>::rot_90_y_cw  = mat<T,R,C>(+static_cast<T>(0.0), +static_cast<T>(0.0), -static_cast<T>(1.0), +static_cast<T>(0.0),
                                                           +static_cast<T>(0.0), +static_cast<T>(1.0), +static_cast<T>(0.0), +static_cast<T>(0.0),
                                                           +static_cast<T>(1.0), +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(0.0),
                                                           +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(1.0));
    template <typename T, size_t R, size_t C>
    const mat<T,R,C> mat<T,R,C>::rot_90_z_cw  = mat<T,R,C>(+static_cast<T>(0.0), +static_cast<T>(1.0), +static_cast<T>(0.0), +static_cast<T>(0.0),
                                                           -static_cast<T>(1.0), +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(0.0),
                                                           +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(1.0), +static_cast<T>(0.0),
                                                           +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(1.0));
    template <typename T, size_t R, size_t C>
    const mat<T,R,C> mat<T,R,C>::rot_90_x_ccw = mat<T,R,C>(+static_cast<T>(1.0), +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(0.0),
                                                           +static_cast<T>(0.0), +static_cast<T>(0.0), -static_cast<T>(1.0), +static_cast<T>(0.0),
                                                           +static_cast<T>(0.0), +static_cast<T>(1.0), +static_cast<T>(0.0), +static_cast<T>(0.0),
                                                           +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(1.0));
    template <typename T, size_t R, size_t C>
    const mat<T,R,C> mat<T,R,C>::rot_90_y_ccw = mat<T,R,C>(+static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(1.0), +static_cast<T>(0.0),
                                                           +static_cast<T>(0.0), +static_cast<T>(1.0), +static_cast<T>(0.0), +static_cast<T>(0.0),
                                                           -static_cast<T>(1.0), +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(0.0),
                                                           +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(1.0));
    template <typename T, size_t R, size_t C>
    const mat<T,R,C> mat<T,R,C>::rot_90_z_ccw = mat<T,R,C>(+static_cast<T>(0.0), -static_cast<T>(1.0), +static_cast<T>(0.0), +static_cast<T>(0.0),
                                                           +static_cast<T>(1.0), +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(0.0),
                                                           +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(1.0), +static_cast<T>(0.0),
                                                           +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(1.0));
    template <typename T, size_t R, size_t C>
    const mat<T,R,C> mat<T,R,C>::rot_180_x    = mat<T,R,C>(+static_cast<T>(1.0), +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(0.0),
                                                           +static_cast<T>(0.0), -static_cast<T>(1.0), +static_cast<T>(0.0), +static_cast<T>(0.0),
                                                           +static_cast<T>(0.0), +static_cast<T>(0.0), -static_cast<T>(1.0), +static_cast<T>(0.0),
                                                           +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(1.0));
    template <typename T, size_t R, size_t C>
    const mat<T,R,C> mat<T,R,C>::rot_180_y    = mat<T,R,C>(-static_cast<T>(1.0), +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(0.0),
                                                           +static_cast<T>(0.0), +static_cast<T>(1.0), +static_cast<T>(0.0), +static_cast<T>(0.0),
                                                           +static_cast<T>(0.0), +static_cast<T>(0.0), -static_cast<T>(1.0), +static_cast<T>(0.0),
                                                           +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(1.0));
    template <typename T, size_t R, size_t C>
    const mat<T,R,C> mat<T,R,C>::rot_180_z    = mat<T,R,C>(-static_cast<T>(1.0), +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(0.0),
                                                           +static_cast<T>(0.0), -static_cast<T>(1.0), +static_cast<T>(0.0), +static_cast<T>(0.0),
                                                           +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(1.0), +static_cast<T>(0.0),
                                                           +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(1.0));
    template <typename T, size_t R, size_t C>
    const mat<T,R,C> mat<T,R,C>::mirror_x     = mat<T,R,C>(-static_cast<T>(1.0), +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(0.0),
                                                           +static_cast<T>(0.0), +static_cast<T>(1.0), +static_cast<T>(0.0), +static_cast<T>(0.0),
                                                           +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(1.0), +static_cast<T>(0.0),
                                                           +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(1.0));
    template <typename T, size_t R, size_t C>
    const mat<T,R,C> mat<T,R,C>::mirror_y     = mat<T,R,C>(+static_cast<T>(1.0), +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(0.0),
                                                           +static_cast<T>(0.0), -static_cast<T>(1.0), +static_cast<T>(0.0), +static_cast<T>(0.0),
                                                           +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(1.0), +static_cast<T>(0.0),
                                                           +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(1.0));
    template <typename T, size_t R, size_t C>
    const mat<T,R,C> mat<T,R,C>::mirror_z     = mat<T,R,C>(+static_cast<T>(1.0), +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(0.0),
                                                           +static_cast<T>(0.0), +static_cast<T>(1.0), +static_cast<T>(0.0), +static_cast<T>(0.0),
                                                           +static_cast<T>(0.0), +static_cast<T>(0.0), -static_cast<T>(1.0), +static_cast<T>(0.0),
                                                           +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(1.0));
    template <typename T, size_t R, size_t C>
    const mat<T,R,C> mat<T,R,C>::zero_x       = mat<T,R,C>(+static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(0.0),
                                                           +static_cast<T>(0.0), +static_cast<T>(1.0), +static_cast<T>(0.0), +static_cast<T>(0.0),
                                                           +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(1.0), +static_cast<T>(0.0),
                                                           +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(1.0));
    template <typename T, size_t R, size_t C>
    const mat<T,R,C> mat<T,R,C>::zero_y       = mat<T,R,C>(+static_cast<T>(1.0), +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(0.0),
                                                           +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(0.0),
                                                           +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(1.0), +static_cast<T>(0.0),
                                                           +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(1.0));
    template <typename T, size_t R, size_t C>
    const mat<T,R,C> mat<T,R,C>::zero_z       = mat<T,R,C>(+static_cast<T>(1.0), +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(0.0),
                                                           +static_cast<T>(0.0), +static_cast<T>(1.0), +static_cast<T>(0.0), +static_cast<T>(0.0),
                                                           +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(0.0),
                                                           +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(0.0), +static_cast<T>(1.0));
    template <typename T, size_t R, size_t C>
    const mat<T,R,C> mat<T,R,C>::yiq_to_rgb   = mat<T,R,C>(+static_cast<T>(1.0),+static_cast<T>( 0.9563),+static_cast<T> (0.6210),
                                                           +static_cast<T>(1.0),+static_cast<T>(-0.2721),+static_cast<T>(-0.6474),
                                                           +static_cast<T>(1.0),+static_cast<T>(-1.1070),+static_cast<T>( 1.7046));

    template <typename T, size_t R, size_t C>
    const mat<T,R,C> mat<T,R,C>::rgb_to_yiq   = mat<T,R,C>(+static_cast<T>(0.299),   +static_cast<T>( 0.587),   +static_cast<T>( 0.114),
                                                           +static_cast<T>(0.595716),+static_cast<T>(-0.274453),+static_cast<T>(-0.321263),
                                                           +static_cast<T>(0.211456),+static_cast<T>(-0.522591),+static_cast<T>( 0.311135));

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
    int compare(const mat<T,R,C>& lhs, const mat<T,R,C>& rhs, const T epsilon = static_cast<T>(0.0)) {
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
    bool isEqual(const mat<T,R,C>& lhs, const mat<T,R,C>& rhs, const T epsilon) {
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
    bool isZero(const mat<T,R,C>& m, const T epsilon) {
        for (size_t c = 0; c < C; ++c) {
            if (!isZero(m[c], epsilon)) {
                return false;
            }
        }
        return true;
    }

    /**
     * Returns a copy of the given matrix.
     *
     * @param m the matrix
     * @return a copy of the given matrix
     */
    template <typename T, size_t R, size_t C>
    mat<T,R,C> operator+(const mat<T,R,C>& m) {
        return m;
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
    vec<T,R> operator*(const vec<T,R>& lhs, const mat<T,R,C>& rhs) {
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
    vec<T,R-1> operator*(const vec<T,R-1>& lhs, const mat<T,R,C>& rhs) {
        return toCartesianCoords(toHomogeneousCoords(lhs) * rhs);
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
    mat<T,S-1,S-1> extractMinor(const mat<T,S,S>& m, const size_t row, const size_t col) {
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
     * Helper struct to compute a matrix determinant. This struct implements a method that works for all S, but
     * there are partial specializations of this template for specific values of S for which faster algorithms exist.
     *
     * @tparam T the component type
     * @tparam S the number of components
     */
    template <typename T, size_t S>
    struct MatrixDeterminant {
        T operator()(const mat<T,S,S>& m) const {
            // Laplace after first col
            MatrixDeterminant<T,S-1> determinant;

            auto result = static_cast<T>(0.0);
            for (size_t r = 0; r < S; r++) {
                const auto f = static_cast<T>(r % 2 == 0 ? 1.0 : -1.0);
                result += f * m[0][r] * determinant(extractMinor(m, r, 0));
            }
            return result;
        }
    };

    // TODO: implement faster block-matrix based method for NxN matrices where N = 2^n

    /**
     * Partial specialization to optimize for the case of a 3x3 matrix.
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
     * Partial specialization to optimize for the case of a 2x2 matrix.
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
     * Partial specialization to optimize for the case of a 1x1 matrix.
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
     * Finds an LUP decomposition of matrix a.
     *
     * Give A, finds P,L,U satisfying PA=LU where P is a permutation matrix,
     * where L is lower-triangular with the diagonal elements set to 1,
     * U is upper-triangular.
     *
     * The permutation matrix is returned in a compressed form where each element of the vector represents a row of
     * the permutation matrix, and a value of `i` means the `i`th column of that row is set to 1.
     *
     * From "LUP-Decomposition", Introduction to Algorithms by Cormen et. al., 2nd. ed. p752.
     *
     * @tparam T the component type
     * @tparam S the number of components
     * @param a the matrix to decompose
     * @return {true, L and U packed into a single matrix, compressed permutation matrix}
     *         or {false, unspecified, unspecified} if a decomposition doesn't exist.
     */
    template <typename T, size_t S>
    std::tuple<bool, mat<T,S,S>, vec<size_t,S>> lupDecomposition(mat<T,S,S> a) {
        using std::swap;

        vec<size_t,S> pi;
        for (size_t i=0; i<S; ++i) {
            pi[i] = i;
        }
        for (size_t k=0; k<S; ++k) {
            T p(0);
            size_t kPrime = 0;
            for (size_t i=k; i<S; ++i) {
                if (vm::abs(a[k][i]) > p) {
                    p = vm::abs(a[k][i]);
                    kPrime = i;
                }
            }
            if (p == 0) {
                return {false, {}, {}};
            }
            swap(pi[k], pi[kPrime]);
            for (size_t i=0; i<S; ++i) {
                swap(a[i][k], a[i][kPrime]);
            }
            for (size_t i=k+1; i<S; ++i) {
                a[k][i] = a[k][i] / a[k][k];
                for (size_t j=k+1; j<S; ++j) {
                    a[j][i] = a[j][i] - a[k][i] * a[j][k];
                }
            }
        }
        return {true, a, pi};
    }

    /**
     * Solves a system of equations given an LUP factorization.
     *
     * From "LUP-Solve", Introduction to Algorithms by Cormen et. al., 2nd. ed. p745.
     *
     * @tparam T the component type
     * @tparam S the number of components
     * @param lu the LU factorization packed into a single matrix; see lupDecomposition()
     * @param pi the permutation matrix packed into a vector; see lupDecomposition()
     * @param b the target value in the system of equations a*x=b
     * @return the solution value x in the system of equations a*x=b
     */
    template <typename T, size_t S>
    vec<T,S> lupSolveInternal(const mat<T,S,S>& lu, const vec<size_t,S>& pi, const vec<T,S>& b) {
        vec<T,S> x;
        vec<T,S> y;
        for (size_t i=0; i<S; ++i) {
            T sum = T(0);
            for (size_t j=0; j+1<=i; ++j) {
                sum += lu[j][i] * y[j];
            }
            y[i] = b[pi[i]] - sum;
        }
        for (size_t i=S-1; i<S; --i) {
            T sum = T(0);
            for (size_t j=i+1; j<S; ++j) {
                sum += lu[j][i] * x[j];
            }
            x[i] = (y[i] - sum) / lu[i][i];
        }
        return x;
    }

    /**
     * Solves a system of equations expressed as a*x=b, using LU factorization with pivoting.
     *
     * @tparam T the component type
     * @tparam S the number of components
     * @param a square matrix
     * @param b column vector
     * @return either {true, x such that a*x=b} or {false, unspecified} if no solution could be found
     */
    template <typename T, size_t S>
    std::tuple<bool, vec<T,S>> lupSolve(const mat<T,S,S>& a, const vec<T,S>& b) {
        auto [success, lu, pi] = lupDecomposition(a);
        if (!success) {
            return {false, {}};
        }
        return {true, lupSolveInternal(lu, pi, b)};
    }
}

#endif
