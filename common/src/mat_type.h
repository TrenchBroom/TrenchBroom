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

#ifndef TRENCHBROOM_MAT_TYPE_H
#define TRENCHBROOM_MAT_TYPE_H

#include "vec_decl.h"

#include <cassert>
#include <vector>

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

    using List = std::vector<mat<T,R,C>>;
    
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
    mat(const T v11, const T v12, const T v13, const T v14,
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

#endif
