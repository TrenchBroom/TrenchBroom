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

#include <gtest/gtest.h>

#include <vecmath/mat.h>
#include <vecmath/vec.h>
#include <vecmath/quat.h>
#include "TrenchBroom.h"
#include "TestUtils.h"

#include <cstdlib>
#include <ctime>

namespace vm {
    TEST(MatTest, zeroMatrix) {
        ASSERT_EQ(
            mat4x4f(
                0, 0, 0, 0,
                0, 0, 0, 0,
                0, 0, 0, 0,
                0, 0, 0, 0),
            mat4x4f::zero);
    }

    TEST(MatTest, identityMatrix) {
        ASSERT_EQ(
            mat4x4f(
                1, 0, 0, 0,
                0, 1, 0, 0,
                0, 0, 1, 0,
                0, 0, 0, 1),
            mat4x4f::identity);
        ASSERT_EQ(
            mat4x4f(
                1, 0, 0, 0,
                0, 1, 0, 0,
                0, 0, 1, 0,
                0, 0, 0, 1),
            mat4x4f::identityMatrix());
    }

    TEST(MatTest, rot90XCWMatrix) {
        const mat4x4d& m = mat4x4d::rot_90_x_cw;
        const vec4d& v = vec4d::pos_y;
        ASSERT_VEC_EQ(vec4d::neg_z, m * v);
    }
    
    TEST(MatTest, rot90YCWMatrix) {
        const mat4x4d& m = mat4x4d::rot_90_y_cw;
        const vec4d& v = vec4d::pos_x;
        ASSERT_VEC_EQ(vec4d::pos_z, m * v);
    }
    
    TEST(MatTest, rot90ZCWMatrix) {
        const mat4x4d& m = mat4x4d::rot_90_z_cw;
        const vec4d& v = vec4d::pos_y;
        ASSERT_VEC_EQ(vec4d::pos_x, m * v);
    }
    
    TEST(MatTest, rot90XCCWMatrix) {
        const mat4x4d& m = mat4x4d::rot_90_x_ccw;
        const vec4d& v = vec4d::pos_y;
        ASSERT_VEC_EQ(vec4d::pos_z, m * v);
    }
    
    TEST(MatTest, rot90YCCWMatrix) {
        const mat4x4d& m = mat4x4d::rot_90_y_ccw;
        const vec4d& v = vec4d::pos_x;
        ASSERT_VEC_EQ(vec4d::neg_z, m * v);
    }
    
    TEST(MatTest, rot90ZCCWMatrix) {
        const mat4x4d& m = mat4x4d::rot_90_z_ccw;
        const vec4d& v = vec4d::pos_x;
        ASSERT_VEC_EQ(vec4d::pos_y, m * v);
    }
    
    TEST(MatTest, rot180XMatrix) {
        const mat4x4d& m = mat4x4d::rot_180_x;
        const vec4d& v = vec4d::pos_y;
        ASSERT_VEC_EQ(vec4d::neg_y, m * v);
    }
    
    TEST(MatTest, rot180YMatrix) {
        const mat4x4d& m = mat4x4d::rot_180_y;
        const vec4d& v = vec4d::pos_x;
        ASSERT_VEC_EQ(vec4d::neg_x, m * v);
    }
    
    TEST(MatTest, rot180ZMatrix) {
        const mat4x4d& m = mat4x4d::rot_180_z;
        const vec4d& v = vec4d::pos_y;
        ASSERT_VEC_EQ(vec4d::neg_y, m * v);
    }
    
    TEST(MatTest, mirXMatrix) {
        const mat4x4d& m = mat4x4d::mirror_x;
        const vec4d v(1.0, 1.0, 1.0, 0.0);
        ASSERT_VEC_EQ(vec4d(-1.0, 1.0, 1.0, 0.0), m * v);
    }
    
    TEST(MatTest, mirYMatrix) {
        const mat4x4d& m = mat4x4d::mirror_y;
        const vec4d v(1.0, 1.0, 1.0, 0.0);
        ASSERT_VEC_EQ(vec4d(1.0, -1.0, 1.0, 0.0), m * v);
    }
    
    TEST(MatTest, mirZMatrix) {
        const mat4x4d& m = mat4x4d::mirror_z;
        const vec4d v(1.0, 1.0, 1.0, 0.0);
        ASSERT_VEC_EQ(vec4d(1.0, 1.0, -1.0, 0.0), m * v);
    }

    TEST(MatTest, zeroXMatrix) {
        const mat4x4d& m = mat4x4d::zero_x;
        const vec4d v(1.0, 1.0, 1.0, 1.0);
        ASSERT_VEC_EQ(vec4d(0.0, 1.0, 1.0, 1.0), m * v);
    }

    TEST(MatTest, zeroYMatrix) {
        const mat4x4d& m = mat4x4d::zero_y;
        const vec4d v(1.0, 1.0, 1.0, 1.0);
        ASSERT_VEC_EQ(vec4d(1.0, 0.0, 1.0, 1.0), m * v);
    }

    TEST(MatTest, zeroZMatrix) {
        const mat4x4d& m = mat4x4d::zero_z;
        const vec4d v(1.0, 1.0, 1.0, 1.0);
        ASSERT_VEC_EQ(vec4d(1.0, 1.0, 0.0, 1.0), m * v);
    }

    TEST(MatTest, fill) {
        ASSERT_EQ(
            mat3x3d(
                3, 3, 3,
                3, 3, 3,
                3, 3, 3),
            mat3x3d::fill(3.0));
    }

    TEST(MatTest, defaultConstructor) {
        const mat4x4d m;
        ASSERT_MAT_EQ(mat4x4d::identity, m);
    }

    TEST(MatTest, conversionConstructor) {
        const mat4x4d from(
            1, 2, 3, 4,
            5, 6, 7, 8,
            7, 6, 5, 4,
            3, 2, 1, 0);
        const mat4x4f to(
            1, 2, 3, 4,
            5, 6, 7, 8,
            7, 6, 5, 4,
            3, 2, 1, 0);
        ASSERT_EQ(to, mat4x4f(from));
    }

    TEST(MatTest, 3x3Constructor) {
        const mat3x3d m(
            1.0, 2.0, 3.0,
            4.0, 5.0, 6.0,
            7.0, 8.0, 9.0);
        ASSERT_DOUBLE_EQ(1.0, m[0][0]);
        ASSERT_DOUBLE_EQ(2.0, m[1][0]);
        ASSERT_DOUBLE_EQ(3.0, m[2][0]);
        ASSERT_DOUBLE_EQ(4.0, m[0][1]);
        ASSERT_DOUBLE_EQ(5.0, m[1][1]);
        ASSERT_DOUBLE_EQ(6.0, m[2][1]);
        ASSERT_DOUBLE_EQ(7.0, m[0][2]);
        ASSERT_DOUBLE_EQ(8.0, m[1][2]);
        ASSERT_DOUBLE_EQ(9.0, m[2][2]);
    }
    
    TEST(MatTest, 4x4Constructor) {
        const mat4x4d m( 1.0,  2.0,  3.0,  4.0,
                         5.0,  6.0,  7.0,  8.0,
                         9.0, 10.0, 11.0, 12.0,
                        13.0, 14.0, 15.0, 16.0);
        ASSERT_DOUBLE_EQ( 1.0, m[0][0]);
        ASSERT_DOUBLE_EQ( 2.0, m[1][0]);
        ASSERT_DOUBLE_EQ( 3.0, m[2][0]);
        ASSERT_DOUBLE_EQ( 4.0, m[3][0]);
        ASSERT_DOUBLE_EQ( 5.0, m[0][1]);
        ASSERT_DOUBLE_EQ( 6.0, m[1][1]);
        ASSERT_DOUBLE_EQ( 7.0, m[2][1]);
        ASSERT_DOUBLE_EQ( 8.0, m[3][1]);
        ASSERT_DOUBLE_EQ( 9.0, m[0][2]);
        ASSERT_DOUBLE_EQ(10.0, m[1][2]);
        ASSERT_DOUBLE_EQ(11.0, m[2][2]);
        ASSERT_DOUBLE_EQ(12.0, m[3][2]);
        ASSERT_DOUBLE_EQ(13.0, m[0][3]);
        ASSERT_DOUBLE_EQ(14.0, m[1][3]);
        ASSERT_DOUBLE_EQ(15.0, m[2][3]);
        ASSERT_DOUBLE_EQ(16.0, m[3][3]);
    }

    TEST(MatTest, subscript) {
        const mat4x4d m(
            1, 2, 3, 4,
            5, 6, 7, 8,
            7, 6, 5, 4,
            3, 2, 1, 0);
        for (size_t c = 0; c < 4; ++c) {
            for (size_t r = 0; r < 4; ++r) {
                ASSERT_DOUBLE_EQ(m.v[c][r], m[c][r]);
            }
        }
    }

    TEST(MatTest, compare) {
        ASSERT_TRUE(
            compare(
                mat4x4d(
                    1, 2, 3, 4,
                    1, 2, 3, 4,
                    1, 2, 3, 4,
                    1, 2, 3, 4),
                mat4x4d(
                    1, 2, 3, 4,
                    1, 2, 3, 4,
                    1, 2, 3, 4,
                    1, 2, 3, 4)
            ) == 0
        );

        ASSERT_TRUE(
            compare(
                mat4x4d(
                    1, 2, 3, 1,
                    1, 2, 3, 4,
                    1, 2, 3, 4,
                    1, 2, 3, 4),
                mat4x4d(
                    1, 2, 3, 4,
                    1, 2, 3, 4,
                    1, 2, 3, 4,
                    1, 2, 3, 4)
            ) < 0
        );

        ASSERT_TRUE(
            compare(
                mat4x4d(
                    1, 2, 3, 5,
                    1, 2, 3, 4,
                    1, 2, 3, 4,
                    1, 2, 3, 4),
                mat4x4d(
                    1, 2, 3, 4,
                    1, 2, 3, 4,
                    1, 2, 3, 4,
                    1, 2, 3, 4)
            ) > 0
        );
    }

    TEST(MatTest, equal) {
        const mat4x4d m( 1,  2,  3,  4,
                         5,  6,  7,  8,
                         9, 10, 11, 12,
                        13, 14, 15, 16);
        const mat4x4d n( 1,  2,  3,  4,
                         5,  6,  7,  8,
                         9, 10, 11, 12,
                        13, 14, 15, 16);
        const mat4x4d o( 2,  2,  3,  4,
                         5,  6,  7,  8,
                         9, 10, 11, 12,
                        13, 14, 15, 16);
        ASSERT_TRUE(m == n);
        ASSERT_FALSE(m == o);
    }

    TEST(MatTest, notEqual) {
        const mat4x4d m( 1,  2,  3,  4,
                         5,  6,  7,  8,
                         9, 10, 11, 12,
                         13, 14, 15, 16);
        const mat4x4d n( 1,  2,  3,  4,
                         5,  6,  7,  8,
                         9, 10, 11, 12,
                         13, 14, 15, 16);
        const mat4x4d o( 2,  2,  3,  4,
                         5,  6,  7,  8,
                         9, 10, 11, 12,
                         13, 14, 15, 16);
        ASSERT_FALSE(m != n);
        ASSERT_TRUE(m != o);
    }

    TEST(MatTest, isEqual) {
        ASSERT_TRUE(
            isEqual(
                mat4x4d(
                    1, 2, 3, 4,
                    1, 2, 3, 4,
                    1, 2, 3, 4,
                    1, 2, 3, 4),
                mat4x4d(
                    1, 2, 3, 4,
                    1, 2, 3, 4,
                    1, 2, 3, 4,
                    1, 2, 3, 4),
                0.0
            )
        );
        ASSERT_TRUE(
            isEqual(
                mat4x4d(
                    1, 2, 3, 4,
                    1, 2, 3, 4,
                    1, 2, 3, 4,
                    1, 2, 3, 4),
                mat4x4d(
                    1, 2, 3, 4,
                    1, 2, 3, 4,
                    1, 2, 3, 4,
                    1, 2, 3, 4),
                0.1
            )
        );
        ASSERT_TRUE(
            isEqual(
                mat4x4d(
                    1.0, 2.0, 3.0, 4.0,
                    1.0, 2.0, 3.0, 4.0,
                    1.0, 2.0, 3.0, 4.0,
                    1.0, 2.0, 3.0, 4),
                mat4x4d(
                    1.1, 2.0, 3.0, 4.0,
                    1.0, 2.0, 3.0, 4.0,
                    1.0, 2.0, 3.0, 4.0,
                    1.0, 2.0, 3.0, 4.0),
                0.11
            )
        );
        ASSERT_TRUE(
            isEqual(
                mat4x4d(
                    1.0, 2.0, 3.0, 4.0,
                    1.0, 2.0, 3.0, 4.0,
                    1.0, 2.0, 3.0, 4.0,
                    1.0, 2.0, 3.0, 4),
                mat4x4d(
                    1.1, 2.0, 3.0, 4.0,
                    1.0, 2.0, 3.0, 4.0,
                    1.0, 2.0, 3.0, 4.0,
                    1.0, 2.0, 3.0, 4.0),
                0.1
            )
        );
        ASSERT_FALSE(
            isEqual(
                mat4x4d(
                    1.0, 2.0, 3.0, 4.0,
                    1.0, 2.0, 3.0, 4.0,
                    1.0, 2.0, 3.0, 4.0,
                    1.0, 2.0, 3.0, 4),
                mat4x4d(
                    1.11, 2.0, 3.0, 4.0,
                    1.0, 2.0, 3.0, 4.0,
                    1.0, 2.0, 3.0, 4.0,
                    1.0, 2.0, 3.0, 4.0),
                0.1
            )
        );
    }

    TEST(MatTest, isZero) {
        ASSERT_TRUE(isZero(mat4x4d::zero, vm::Cd::almostZero()));
        ASSERT_FALSE(isZero(mat4x4d::identity, vm::Cd::almostZero()));
    }

    TEST(MatTest, negate) {
        ASSERT_EQ(
            +mat4x4d(
                -1, -2, -3, -4,
                -1, -2, -3, -4,
                -1, -2, -3, -4,
                -1, -2, -3, -4),
            -mat4x4d(
                +1, +2, +3, +4,
                +1, +2, +3, +4,
                +1, +2, +3, +4,
                +1, +2, +3, +4)
        );
    }

    TEST(MatTest, add) {
        const mat4x4d m( 1.0,  2.0,  3.0,  4.0,
                         5.0,  6.0,  7.0,  8.0,
                         9.0, 10.0, 11.0, 12.0,
                        13.0, 14.0, 15.0, 16.0);
        const mat4x4d n( 2.0,  2.0,  3.0,  4.0,
                         5.0,  8.0,  7.0,  8.0,
                         9.0, 11.0, 11.0, 12.0,
                        13.0, 14.0, 15.0, 16.0);
        const mat4x4d o = m + n;
    
        for (size_t c = 0; c < 4; ++c) {
            for (size_t r = 0; r < 4; ++r) {
                ASSERT_DOUBLE_EQ(m[c][r] + n[c][r], o[c][r]);
            }
        }
    }

    TEST(MatTest, subtract) {
        const mat4x4d m( 1.0,  2.0,  3.0,  4.0,
                         5.0,  6.0,  7.0,  8.0,
                         9.0, 10.0, 11.0, 12.0,
                        13.0, 14.0, 15.0, 16.0);
        const mat4x4d n( 2.0,  2.0,  3.0,  4.0,
                         5.0,  8.0,  7.0,  8.0,
                         9.0, 11.0, 11.0, 12.0,
                        13.0, 14.0, 15.0, 16.0);
        const mat4x4d o = m - n;
    
        for (size_t c = 0; c < 4; ++c) {
            for (size_t r = 0; r < 4; ++r) {
                ASSERT_DOUBLE_EQ(m[c][r] - n[c][r], o[c][r]);
            }
        }
    }
    
    TEST(MatTest, multiply) {
        const mat4x4d m( 1.0,  2.0,  3.0,  4.0,
                         5.0,  6.0,  7.0,  8.0,
                         9.0, 10.0, 11.0, 12.0,
                        13.0, 14.0, 15.0, 16.0);
        const mat4x4d n( 2.0,  2.0,  3.0,  4.0,
                         5.0,  8.0,  7.0,  8.0,
                         9.0, 11.0, 11.0, 12.0,
                        13.0, 14.0, 15.0, 16.0);
        const mat4x4d r( 91.0, 107.0, 110.0, 120.0,
                        207.0, 247.0, 254.0, 280.0,
                        323.0, 387.0, 398.0, 440.0,
                        439.0, 527.0, 542.0, 600.0);
        const mat4x4d o = m * n;
        ASSERT_MAT_EQ(r, o);
    }
    
    TEST(MatTest, scalarMultiply_right) {
        const mat4x4d m( 1.0,  2.0,  3.0,  4.0,
                         5.0,  6.0,  7.0,  8.0,
                         9.0, 10.0, 11.0, 12.0,
                        13.0, 14.0, 15.0, 16.0);
        const mat4x4d o = m * 3.0;
    
        for (size_t c = 0; c < 4; ++c) {
            for (size_t r = 0; r < 4; ++r) {
                ASSERT_DOUBLE_EQ(m[c][r] * 3.0, o[c][r]);
            }
        }
    }
    
    TEST(MatTest, scalarMultiply_left) {
        const mat4x4d m( 1.0,  2.0,  3.0,  4.0,
                         5.0,  6.0,  7.0,  8.0,
                         9.0, 10.0, 11.0, 12.0,
                         13.0, 14.0, 15.0, 16.0);
        const mat4x4d o = 3.0 * m;
    
        for (size_t c = 0; c < 4; ++c) {
            for (size_t r = 0; r < 4; ++r) {
                ASSERT_DOUBLE_EQ(m[c][r] * 3.0, o[c][r]);
            }
        }
    }
    
    TEST(MatTest, divide) {
        const mat4x4d m( 1.0,  2.0,  3.0,  4.0,
                         5.0,  6.0,  7.0,  8.0,
                         9.0, 10.0, 11.0, 12.0,
                        13.0, 14.0, 15.0, 16.0);
        const mat4x4d o = m / 3.0;
    
        for (size_t c = 0; c < 4; ++c) {
            for (size_t r = 0; r < 4; ++r) {
                ASSERT_DOUBLE_EQ(m[c][r] / 3.0, o[c][r]);
            }
        }
    }
    
    TEST(MatTest, vectorMultiply_right) {
        const vec4d v(1.0, 2.0, 3.0, 1.0);
        ASSERT_VEC_EQ(v, mat4x4d::identity * v);

        const mat4x4d m( 1.0,  2.0,  3.0,  4.0,
                         5.0,  6.0,  7.0,  8.0,
                         9.0, 10.0, 11.0, 12.0,
                         13.0, 14.0, 15.0, 16.0);
        const vec4d r(18.0, 46.0, 74.0, 102.0);
        ASSERT_VEC_EQ(r, m * v);
    }
    
    TEST(MatTest, vectorMultiply_left) {
        const vec4d v(1.0, 2.0, 3.0, 1.0);
        ASSERT_VEC_EQ(v, v * mat4x4d::identity);

        const mat4x4d m( 1.0,  2.0,  3.0,  4.0,
                         5.0,  6.0,  7.0,  8.0,
                         9.0, 10.0, 11.0, 12.0,
                         13.0, 14.0, 15.0, 16.0);
        const vec4d r(51.0, 58.0, 65.0, 72.0);
        ASSERT_VEC_EQ(r, v * m);
    }
    
    TEST(MatTest, vectorMultiply_right_2) {
        const vec3d v(1.0, 2.0, 3.0);
        const mat4x4d m( 1.0,  2.0,  3.0,  4.0,
                         5.0,  6.0,  7.0,  8.0,
                         9.0, 10.0, 11.0, 12.0,
                        13.0, 14.0, 15.0, 16.0);
        const vec4d r(18.0, 46.0, 74.0, 102.0);
        ASSERT_VEC_EQ(toCartesianCoords(r), m * v);
    }
    
    TEST(MatTest, vectorMultiply_left_2) {
        const vec3d v(1.0, 2.0, 3.0);
        const mat4x4d m( 1.0,  2.0,  3.0,  4.0,
                         5.0,  6.0,  7.0,  8.0,
                         9.0, 10.0, 11.0, 12.0,
                        13.0, 14.0, 15.0, 16.0);
        const vec4d r(51.0, 58.0, 65.0, 72.0);
        ASSERT_VEC_EQ(toCartesianCoords(r), v * m);
    }
    

    TEST(MatTest, transpose) {
        mat<double, 4, 4> m;
        for (size_t c = 0; c < 4; ++c) {
            for (size_t r = 0; r < 4; ++r) {
                m[c][r] = static_cast<double>(c * 4 + r);
            }
        }
    
        const auto t = transpose(m);
    
        for (size_t c = 0; c < 4; ++c) {
            for (size_t r = 0; r < 4; ++r) {
                ASSERT_DOUBLE_EQ(m[c][r], t[r][c]);
            }
        }
    }
    
    TEST(MatTest, extractMinor) {
        const mat4x4d m( 1.0,  2.0,  3.0,  4.0,
                         5.0,  6.0,  7.0,  8.0,
                         9.0, 10.0, 11.0, 12.0,
                        13.0, 14.0, 15.0, 16.0);
        const mat3x3d m00( 6.0,  7.0,  8.0,
                          10.0, 11.0, 12.0,
                          14.0, 15.0, 16.0);
        const mat3x3d m33( 1.0,  2.0,  3.0,
                           5.0,  6.0,  7.0,
                           9.0, 10.0, 11.0);
        const mat3x3d m12( 1.0,  2.0,  4.0,
                           9.0, 10.0, 12.0,
                          13.0, 14.0, 16.0);
        const mat3x3d m21( 1.0,  3.0,  4.0,
                           5.0,  7.0,  8.0,
                          13.0, 15.0, 16.0);
        ASSERT_MAT_EQ(m00, extractMinor(m, 0, 0));
        ASSERT_MAT_EQ(m33, extractMinor(m, 3, 3));
        ASSERT_MAT_EQ(m12, extractMinor(m, 1, 2));
        ASSERT_MAT_EQ(m21, extractMinor(m, 2, 1));
    }
    
    TEST(MatTest, determinant) {
        const mat4x4d m1( 1.0,  2.0,  3.0,  4.0,
                          5.0,  6.0,  7.0,  8.0,
                          9.0, 10.0, 11.0, 12.0,
                         13.0, 14.0, 15.0, 16.0);
        const mat4x4d m2(65.0, 12.0, -3.0, -5.0,
                         -5.0,  1.0,  0.0,  0.0,
                         19.0, 10.0, 11.0,  8.0,
                          0.0,  1.0, -8.0,  3.0);
        const mat4x4d m3( 3.0,  2.0, -1.0,  4.0,
                          2.0,  1.0,  5.0,  7.0,
                          0.0,  5.0,  2.0, -6.0,
                         -1.0,  2.0,  1.0,  0.0);
        ASSERT_DOUBLE_EQ(0.0, computeDeterminant(mat4x4d::zero));
        ASSERT_DOUBLE_EQ(1.0, computeDeterminant(mat4x4d::identity));
        ASSERT_DOUBLE_EQ(0.0, computeDeterminant(m1));
        ASSERT_DOUBLE_EQ(15661.0, computeDeterminant(m2));
        ASSERT_DOUBLE_EQ(-418.0, computeDeterminant(m3));
    }
    
    TEST(MatTest, computeAdjugate) {
        const mat4x4d m1( 1.0,  2.0,  3.0,  4.0,
                          5.0,  6.0,  7.0,  8.0,
                          9.0, 10.0, 11.0, 12.0,
                         13.0, 14.0, 15.0, 16.0);
        const mat4x4d m2(65.0, 12.0, -3.0, -5.0,
                         -5.0,  1.0,  0.0,  0.0,
                         19.0, 10.0, 11.0,  8.0,
                          0.0,  1.0, -8.0,  3.0);
        const mat4x4d m3( 3.0,  2.0, -1.0,  4.0,
                          2.0,  1.0,  5.0,  7.0,
                          0.0,  5.0,  2.0, -6.0,
                         -1.0,  2.0,  1.0,  0.0);
        const mat4x4d r1(0.0, 0.0, 0.0, 0.0,
                         0.0, 0.0, 0.0, 0.0,
                         0.0, 0.0, 0.0, 0.0,
                         0.0, 0.0, 0.0, 0.0);
        const mat4x4d r2(  97.0, -1685.0,  49.0,    31.0,
                          485.0,  7236.0, 245.0,	   155.0,
                         -167.0,	  -651.0, 400.0, -1345.0,
                         -607.0, -4148.0, 985.0,  1582.0);
        const mat4x4d r3(-47.0, -28.0, -64.0,  221.0,
                         -56.0,  20.0, -14.0, -128.0,
                          65.0, -68.0, -36.0,   59.0,
                         -25.0,  -6.0,  46.0,  -87.0);
    
        ASSERT_MAT_EQ(mat4x4d::identity, computeAdjugate(mat4x4d::identity));
        ASSERT_MAT_EQ(mat4x4d::zero, computeAdjugate(mat4x4d::zero));
        ASSERT_MAT_EQ(r1, computeAdjugate(m1));
        ASSERT_MAT_EQ(r2, computeAdjugate(m2));
        ASSERT_MAT_EQ(r3, computeAdjugate(m3));
    }
    
    template <typename T, size_t S>
    void ASSERT_INVERTIBLE(const mat<T,S,S>& expected, const mat<T,S,S>& actual) {
        auto [invertible, inverse] = invert(actual);
        ASSERT_MAT_EQ(expected, inverse);
        ASSERT_TRUE(invertible);
    }
    
    template <typename T, size_t S>
    void ASSERT_NOT_INVERTIBLE(const mat<T,S,S>& actual) {
        auto [invertible, inverse] = invert(actual);
        ASSERT_FALSE(invertible); unused(inverse);
    }
    
    TEST(MatTest, invert) {
        const mat4x4d m1( 1.0,  2.0,  3.0,  4.0,
                          5.0,  6.0,  7.0,  8.0,
                          9.0, 10.0, 11.0, 12.0,
                         13.0, 14.0, 15.0, 16.0);
        const mat4x4d m2(65.0, 12.0, -3.0, -5.0,
                         -5.0,  1.0,  0.0,  0.0,
                         19.0, 10.0, 11.0,  8.0,
                          0.0,  1.0, -8.0,  3.0);
        const mat4x4d m3( 0.0, -1.0,  0.0,    0.0,
                          0.0,  0.0,  1.0,  128.0,
                         -1.0,  0.0,  0.0,    0.0,
                          0.0,  0.0,  0.0,    1.0);
        const mat4x4d m4( 0.0,  0.0, -1.0,    0.0,
                         -1.0,  0.0,  0.0,    0.0,
                          0.0,  1.0,  0.0, -128.0,
                          0.0,  0.0,  0.0,    1.0);
        const mat4x4d r2( 0.0061937296468936, -0.10759210778367, 0.0031287912649256, 0.0019794393716876,
                          0.030968648234468,   0.46203946108167, 0.015643956324628,  0.0098971968584382,
                         -0.01066343145393,   -0.04156822680544, 0.025541153183066, -0.08588212757806,
                         -0.038758699955303,  -0.2648617585084,  0.062895089713301,  0.10101526083903);
    
        ASSERT_INVERTIBLE(mat4x4d::identity, mat4x4d::identity);
        ASSERT_INVERTIBLE(r2, m2);
        ASSERT_INVERTIBLE(m4, m3);
        ASSERT_NOT_INVERTIBLE(mat4x4d::zero);
        ASSERT_NOT_INVERTIBLE(m1);
    }

    TEST(MatTest, stripTranslation) {
        const vec3d v(2.0, 3.0, 4.0);
        const mat4x4d t = translationMatrix(v);
        const mat4x4d r = rotationMatrix(toRadians(15.0), toRadians(30.0), toRadians(45.0));
        ASSERT_EQ(r, stripTranslation(r * t));
        ASSERT_EQ(r, stripTranslation(t * r));
    }
}
