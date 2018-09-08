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

#ifndef TRENCHBROOM_MAT_IMPL_H
#define TRENCHBROOM_MAT_IMPL_H

#include "mat_decl.h"
#include "vec_decl.h"
#include "vec_impl.h"
#include "quat_decl.h"
#include "quat_impl.h"

#include <cassert>
#include <tuple>

namespace vm {
    template <typename T, size_t R, size_t C>
    mat<T,R,C> mat<T,R,C>::fill(const T value) {
        mat<T,R,C> result;
        for (size_t c = 0; c < C; c++) {
            result[c] = vec<T,R>::fill(value);
        }
        return result;
    }

    template <typename T, size_t R, size_t C>
    mat<T,R,C> mat<T,R,C>::identityMatrix() {
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

    template <typename T, size_t R, size_t C>
    mat<T,R,C>::mat() {
        for (size_t c = 0; c < C; c++) {
            for (size_t r = 0; r < R; r++) {
                v[c][r] = c == r ? static_cast<T>(1.0) : static_cast<T>(0.0);
            }
        }
    }


    template <typename T, size_t R, size_t C>
    mat<T,R,C>::mat(const T v11, const T v12, const T v13,
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

    template <typename T, size_t R, size_t C>
    mat<T,R,C>::mat(const T v11, const T v12, const T v13, const T v14,
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

    template <typename T, size_t R, size_t C>
    vec<T,R>& mat<T,R,C>::operator[] (const size_t index) {
        assert(index < C);
        return v[index];
    }

    template <typename T, size_t R, size_t C>
    const vec<T,R>& mat<T,R,C>::operator[] (const size_t index) const {
        assert(index < C);
        return v[index];
    }

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

    template <typename T, size_t R, size_t C>
    bool operator==(const mat<T,R,C>& lhs, const mat<T,R,C>& rhs) {
        return compare(lhs, rhs, static_cast<T>(0.0)) == 0;
    }

    template <typename T, size_t R, size_t C>
    bool operator!=(const mat<T,R,C>& lhs, const mat<T,R,C>& rhs) {
        return compare(lhs, rhs, static_cast<T>(0.0)) != 0;
    }

    template <typename T, size_t R, size_t C>
    bool equal(const mat<T,R,C>& lhs, const mat<T,R,C>& rhs, const T epsilon) {
        return compare(lhs, rhs, epsilon) == 0;
    }

    template <typename T, size_t R, size_t C>
    bool isZero(const mat<T,R,C>& m, const T epsilon) {
        for (size_t c = 0; c < C; ++c) {
            if (!isZero(m[c], epsilon)) {
                return false;
            }
        }
        return true;
    }

    template <typename T, size_t R, size_t C>
    mat<T,R,C> operator-(const mat<T,R,C>& m) {
        mat<T,R,C> result;
        for (size_t c = 0; c < C; c++) {
            result[c] = -m[c];
        }
        return result;
    }

    template <typename T, size_t R, size_t C>
    mat<T,R,C> operator+(const mat<T,R,C>& lhs, const mat<T,R,C>& rhs) {
        mat<T,R,C> result;
        for (size_t c = 0; c < C; c++) {
            result[c] = lhs[c] + rhs[c];
        }
        return result;
    }

    template <typename T, size_t R, size_t C>
    mat<T,R,C> operator-(const mat<T,R,C>& lhs, const mat<T,R,C>& rhs) {
        mat<T,R,C> result;
        for (size_t c = 0; c < C; c++) {
            result[c] = lhs[c] - rhs[c];
        }
        return result;
    }

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

    template <typename T, size_t R, size_t C>
    mat<T,R,C> operator*(const mat<T,R,C>& lhs, const T rhs) {
        mat<T,R,C> result;
        for (size_t c = 0; c < C; ++c) {
            result[c] = lhs[c] * rhs;
        }
        return result;
    }

    template <typename T, size_t R, size_t C>
    mat<T,R,C> operator*(const T lhs, const mat<T,R,C>& rhs) {
        return rhs * lhs;
    }

    template <typename T, size_t R, size_t C>
    mat<T,R,C> operator/(const mat<T,R,C>& lhs, const T rhs) {
        mat<T,R,C> result;
        for (size_t c = 0; c < C; ++c) {
            result[c] = lhs[c] / rhs;
        }
        return result;
    }

    template <typename T, size_t R, size_t C>
    vec<T,R> operator*(const vec<T,R>& lhs, const mat<T,R,C>& rhs) {
        vec<T,R> result;
        for (size_t c = 0; c < C; c++) {
            result[c] = dot(lhs, rhs[c]);
        }
        return result;
    }

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

    template <typename T, size_t R, size_t C>
    vec<T,C-1> operator*(const mat<T,R,C>& lhs, const vec<T,C-1>& rhs) {
        return toCartesianCoords(lhs * toHomogeneousCoords(rhs));
    }

    template <typename T, size_t R, size_t C>
    vec<T,R-1> operator*(const vec<T,R-1>& lhs, const mat<T,R,C>& rhs) {
        return toCartesianCoords(toHomogeneousCoords(lhs) * rhs);
    }

    template <typename T, size_t R, size_t C>
    typename vec<T,R>::List operator*(const typename vec<T,R>::List& lhs, const mat<T,R,C>& rhs) {
        typename vec<T,R>::List result;
        result.reserve(lhs.size());
        std::transform(std::begin(lhs), std::end(lhs), std::back_inserter(result), [rhs](const vec<T,R>& elem) { return elem * rhs; });
        return result;
    }

    template <typename T, size_t R, size_t C>
    typename vec<T,R-1>::List operator*(const typename vec<T,R-1>::List& lhs, const mat<T,R,C>& rhs) {
        typename vec<T,R-1>::List result;
        result.reserve(lhs.size());
        std::transform(std::begin(lhs), std::end(lhs), std::back_inserter(result), [rhs](const vec<T,R-1>& elem) { return elem * rhs; });
        return result;
    }

    template <typename T, size_t R, size_t C>
    typename vec<T,C>::List operator*(const mat<T,R,C>& lhs, const typename vec<T,C>::List& rhs) {
        typename vec<T,C>::List result;
        result.reserve(rhs.size());
        std::transform(std::begin(rhs), std::end(rhs), std::back_inserter(result), [lhs](const vec<T,C>& elem) { return lhs * elem; });
        return result;
    }

    template <typename T, size_t R, size_t C>
    typename vec<T,C-1>::List operator*(const mat<T,R,C>& lhs, const typename vec<T,C-1>::List& rhs) {
        typename vec<T,C-1>::List result;
        result.reserve(rhs.size());
        std::transform(std::begin(rhs), std::end(rhs), std::back_inserter(result), [lhs](const vec<T,C-1>& elem) { return lhs * elem; });
        return result;
    }

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

    template <typename T>
    struct MatrixDeterminant<T,2> {
        T operator() (const mat<T,2,2>& m) const {
            return (  m[0][0]*m[1][1]
                      - m[1][0]*m[0][1]);
        }
    };

    template <typename T>
    struct MatrixDeterminant<T,1> {
        T operator() (const mat<T,1,1>& m) const {
            return m[0][0];
        }
    };

    template <typename T, size_t S>
    T computeDeterminant(const mat<T,S,S>& m) {
        return MatrixDeterminant<T,S>()(m);
    }

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

    template <typename T>
    mat<T,4,4> rotationMatrix(const quat<T>& quat) {
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

    template <typename T>
    mat<T,4,4> rotationMatrix(const vec<T,3>& from, const vec<T,3>& to) {
        return rotationMatrix(quat<T>(from, to));
    }

    template <typename T, size_t S>
    mat<T,S+1,S+1> translationMatrix(const vec<T,S>& delta) {
        mat<T,S+1,S+1> translation;
        for (size_t i = 0; i < S; ++i) {
            translation[S][i] = delta[i];
        }
        return translation;
    }

    template <typename T, size_t S>
    mat<T,S,S> translationMatrix(const mat<T,S,S>& m) {
        mat<T,S,S> result;
        for (size_t i = 0; i < S-1; ++i) {
            result[S-1][i] = m[S-1][i];
        }
        return result;
    }

    template <typename T, size_t S>
    mat<T,S,S> stripTranslation(const mat<T,S,S>& m) {
        mat<T,S,S> result(m);
        for (size_t i = 0; i < S-1; ++i) {
            result[S-1][i] = static_cast<T>(0.0);
        }
        return result;
    }

    template <typename T, size_t S>
    mat<T,S+1,S+1> scalingMatrix(const vec<T,S>& factors) {
        mat<T,S+1,S+1> scaling;
        for (size_t i = 0; i < S; ++i) {
            scaling[i][i] = factors[i];
        }
        return scaling;
    }

    template <typename T>
    mat<T,4,4> mirrorMatrix(const Math::Axis::Type axis) {
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

    template <typename T>
    mat<T,4,4> coordinateSystemMatrix(const vec<T,3>& x, const vec<T,3>& y, const vec<T,3>& z, const vec<T,3>& o) {
        [[maybe_unused]] bool invertible;
        mat<T,4,4> result;
        std::tie(invertible, result) = invert(mat<T,4,4>(x[0], y[0], z[0], o[0],
                                                         x[1], y[1], z[1], o[1],
                                                         x[2], y[2], z[2], o[2],
                                                         0.0,  0.0,  0.0,  1.0));
        assert(invertible);
        return result;
    }

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

    template <typename T>
    mat<T,4,4> planeProjectionMatrix(const T distance, const vec<T,3>& normal) {
        return planeProjectionMatrix(distance, normal, normal);
    }

    template <typename T>
    mat<T,4,4> shearMatrix(const T Sxy, const T Sxz, const T Syx, const T Syz, const T Szx, const T Szy) {
        return mat<T,4,4>(1.0, Syx, Szx, 0.0,
                          Sxy, 1.0, Szy, 0.0,
                          Sxz, Syz, 1.0, 0.0,
                          0.0, 0.0, 0.0, 1.0);
    }
}

#endif //TRENCHBROOM_MAT_IMPL_H
