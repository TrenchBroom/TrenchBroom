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

#include "Mat.h"
#include "Quat.h"
#include "vec_type.h"
#include "TrenchBroom.h"
#include "TestUtils.h"

#include <cstdlib>
#include <ctime>

TEST(MatTest, nullMatrix) {
    const Mat4x4d& m = Mat4x4d::Null;
    for (size_t c = 0; c < 4; ++c) {
        for (size_t r = 0; r < 4; ++r) {
            ASSERT_DOUBLE_EQ(0.0, m[c][r]);
        }
    }
}

TEST(MatTest, identityMatrix) {
    const Mat4x4d& m = Mat4x4d::Identity;
    for (size_t c = 0; c < 4; ++c) {
        for (size_t r = 0; r < 4; ++r) {
            if (c == r)
                ASSERT_DOUBLE_EQ(1.0, m[c][r]);
            else
                ASSERT_DOUBLE_EQ(0.0, m[c][r]);
        }
    }
}

TEST(MatTest, rot90XCWMatrix) {
    const Mat4x4d& m = Mat4x4d::Rot90XCW;
    const vec4d& v = vec4d::pos_y;
    ASSERT_VEC_EQ(vec4d::neg_z, m * v);
}

TEST(MatTest, rot90YCWMatrix) {
    const Mat4x4d& m = Mat4x4d::Rot90YCW;
    const vec4d& v = vec4d::pos_x;
    ASSERT_VEC_EQ(vec4d::pos_z, m * v);
}

TEST(MatTest, rot90ZCWMatrix) {
    const Mat4x4d& m = Mat4x4d::Rot90ZCW;
    const vec4d& v = vec4d::pos_y;
    ASSERT_VEC_EQ(vec4d::pos_x, m * v);
}

TEST(MatTest, rot90XCCWMatrix) {
    const Mat4x4d& m = Mat4x4d::Rot90XCCW;
    const vec4d& v = vec4d::pos_y;
    ASSERT_VEC_EQ(vec4d::pos_z, m * v);
}

TEST(MatTest, rot90YCCWMatrix) {
    const Mat4x4d& m = Mat4x4d::Rot90YCCW;
    const vec4d& v = vec4d::pos_x;
    ASSERT_VEC_EQ(vec4d::neg_z, m * v);
}

TEST(MatTest, rot90ZCCWMatrix) {
    const Mat4x4d& m = Mat4x4d::Rot90ZCCW;
    const vec4d& v = vec4d::pos_x;
    ASSERT_VEC_EQ(vec4d::pos_y, m * v);
}

TEST(MatTest, rot180XMatrix) {
    const Mat4x4d& m = Mat4x4d::Rot180X;
    const vec4d& v = vec4d::pos_y;
    ASSERT_VEC_EQ(vec4d::neg_y, m * v);
}

TEST(MatTest, rot180YMatrix) {
    const Mat4x4d& m = Mat4x4d::Rot180Y;
    const vec4d& v = vec4d::pos_x;
    ASSERT_VEC_EQ(vec4d::neg_x, m * v);
}

TEST(MatTest, rot180ZMatrix) {
    const Mat4x4d& m = Mat4x4d::Rot180Z;
    const vec4d& v = vec4d::pos_y;
    ASSERT_VEC_EQ(vec4d::neg_y, m * v);
}

TEST(MatTest, mirXMatrix) {
    const Mat4x4d& m = Mat4x4d::MirX;
    const vec4d v(1.0, 1.0, 1.0, 0.0);
    ASSERT_VEC_EQ(vec4d(-1.0, 1.0, 1.0, 0.0), m * v);
}

TEST(MatTest, mirYMatrix) {
    const Mat4x4d& m = Mat4x4d::MirY;
    const vec4d v(1.0, 1.0, 1.0, 0.0);
    ASSERT_VEC_EQ(vec4d(1.0, -1.0, 1.0, 0.0), m * v);
}

TEST(MatTest, mirZMatrix) {
    const Mat4x4d& m = Mat4x4d::MirZ;
    const vec4d v(1.0, 1.0, 1.0, 0.0);
    ASSERT_VEC_EQ(vec4d(1.0, 1.0, -1.0, 0.0), m * v);
}

TEST(MatTest, defaultConstructor) {
    const Mat4x4d m;
    ASSERT_MAT_EQ(Mat4x4d::Identity, m);
}

TEST(MatTest, 3x3Constructor) {
    const Mat3x3d m(1.0, 2.0, 3.0,
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
    const Mat4x4d m( 1.0,  2.0,  3.0,  4.0,
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

TEST(MatTest, copyConstructor) {
    const Mat4x4d m( 1.0,  2.0,  3.0,  4.0,
                    5.0,  6.0,  7.0,  8.0,
                    9.0, 10.0, 11.0, 12.0,
                    13.0, 14.0, 15.0, 16.0);
    const Mat4x4d n(m);
    ASSERT_MAT_EQ(m, n);
}

TEST(MatTest, assign) {
    const Mat4x4d m( 1.0,  2.0,  3.0,  4.0,
                    5.0,  6.0,  7.0,  8.0,
                    9.0, 10.0, 11.0, 12.0,
                    13.0, 14.0, 15.0, 16.0);
    const Mat4x4d n = m;
    ASSERT_MAT_EQ(m, n);
}

TEST(MatTest, negate) {
    const Mat4x4d m( 1.0,  2.0,  3.0,  4.0,
                    5.0,  6.0,  7.0,  8.0,
                    9.0, 10.0, 11.0, 12.0,
                    13.0, 14.0, 15.0, 16.0);
    const Mat4x4d n = -m;

    for (size_t c = 0; c < 4; ++c) {
        for (size_t r = 0; r < 4; ++r) {
            ASSERT_DOUBLE_EQ(-m[c][r], n[c][r]);
        }
    }
}

TEST(MatTest, equality) {
    const Mat4x4d m( 1.0,  2.0,  3.0,  4.0,
                     5.0,  6.0,  7.0,  8.0,
                     9.0, 10.0, 11.0, 12.0,
                    13.0, 14.0, 15.0, 16.0);
    const Mat4x4d n( 1.0,  2.0,  3.0,  4.0,
                     5.0,  6.0,  7.0,  8.0,
                     9.0, 10.0, 11.0, 12.0,
                    13.0, 14.0, 15.0, 16.0);
    const Mat4x4d o( 2.0,  2.0,  3.0,  4.0,
                     5.0,  6.0,  7.0,  8.0,
                     9.0, 10.0, 11.0, 12.0,
                    13.0, 14.0, 15.0, 16.0);
    ASSERT_TRUE(m == n);
    ASSERT_FALSE(m == o);
}

TEST(MatTest, addMatrix) {
    const Mat4x4d m( 1.0,  2.0,  3.0,  4.0,
                     5.0,  6.0,  7.0,  8.0,
                     9.0, 10.0, 11.0, 12.0,
                    13.0, 14.0, 15.0, 16.0);
    const Mat4x4d n( 2.0,  2.0,  3.0,  4.0,
                     5.0,  8.0,  7.0,  8.0,
                     9.0, 11.0, 11.0, 12.0,
                    13.0, 14.0, 15.0, 16.0);
    const Mat4x4d o = m + n;

    for (size_t c = 0; c < 4; ++c) {
        for (size_t r = 0; r < 4; ++r) {
            ASSERT_DOUBLE_EQ(m[c][r] + n[c][r], o[c][r]);
        }
    }
}

TEST(MatTest, subtractMatrix) {
    const Mat4x4d m( 1.0,  2.0,  3.0,  4.0,
                     5.0,  6.0,  7.0,  8.0,
                     9.0, 10.0, 11.0, 12.0,
                    13.0, 14.0, 15.0, 16.0);
    const Mat4x4d n( 2.0,  2.0,  3.0,  4.0,
                     5.0,  8.0,  7.0,  8.0,
                     9.0, 11.0, 11.0, 12.0,
                    13.0, 14.0, 15.0, 16.0);
    const Mat4x4d o = m - n;

    for (size_t c = 0; c < 4; ++c) {
        for (size_t r = 0; r < 4; ++r) {
            ASSERT_DOUBLE_EQ(m[c][r] - n[c][r], o[c][r]);
        }
    }
}

TEST(MatTest, multiplyWithMatrix) {
    const Mat4x4d m( 1.0,  2.0,  3.0,  4.0,
                     5.0,  6.0,  7.0,  8.0,
                     9.0, 10.0, 11.0, 12.0,
                    13.0, 14.0, 15.0, 16.0);
    const Mat4x4d n( 2.0,  2.0,  3.0,  4.0,
                     5.0,  8.0,  7.0,  8.0,
                     9.0, 11.0, 11.0, 12.0,
                    13.0, 14.0, 15.0, 16.0);
    const Mat4x4d r( 91.0, 107.0, 110.0, 120.0,
                    207.0, 247.0, 254.0, 280.0,
                    323.0, 387.0, 398.0, 440.0,
                    439.0, 527.0, 542.0, 600.0);
    const Mat4x4d o = m * n;
    ASSERT_MAT_EQ(r, o);
}

TEST(MatTest, rightMultiplyWithScalar) {
    const Mat4x4d m( 1.0,  2.0,  3.0,  4.0,
                     5.0,  6.0,  7.0,  8.0,
                     9.0, 10.0, 11.0, 12.0,
                    13.0, 14.0, 15.0, 16.0);
    const Mat4x4d o = m * 3.0;

    for (size_t c = 0; c < 4; ++c) {
        for (size_t r = 0; r < 4; ++r) {
            ASSERT_DOUBLE_EQ(m[c][r] * 3.0, o[c][r]);
        }
    }
}

TEST(MatTest, leftMultiplyWithScalar) {
    const Mat4x4d m( 1.0,  2.0,  3.0,  4.0,
                    5.0,  6.0,  7.0,  8.0,
                    9.0, 10.0, 11.0, 12.0,
                    13.0, 14.0, 15.0, 16.0);
    const Mat4x4d o = 3.0 * m;

    for (size_t c = 0; c < 4; ++c) {
        for (size_t r = 0; r < 4; ++r) {
            ASSERT_DOUBLE_EQ(m[c][r] * 3.0, o[c][r]);
        }
    }
}

TEST(MatTest, divideByScalar) {
    const Mat4x4d m( 1.0,  2.0,  3.0,  4.0,
                     5.0,  6.0,  7.0,  8.0,
                     9.0, 10.0, 11.0, 12.0,
                    13.0, 14.0, 15.0, 16.0);
    const Mat4x4d o = m / 3.0;

    for (size_t c = 0; c < 4; ++c) {
        for (size_t r = 0; r < 4; ++r) {
            ASSERT_DOUBLE_EQ(m[c][r] / 3.0, o[c][r]);
        }
    }
}

TEST(MatTest, rightMultiplyIdentityMatrixWithVector) {
    const vec4d v(1.0, 2.0, 3.0, 1.0);
    ASSERT_VEC_EQ(v, Mat4x4d::Identity * v);
}

TEST(MatTest, rightMultiplyWithVector) {
    const vec4d v(1.0, 2.0, 3.0, 1.0);
    const Mat4x4d m( 1.0,  2.0,  3.0,  4.0,
                     5.0,  6.0,  7.0,  8.0,
                     9.0, 10.0, 11.0, 12.0,
                    13.0, 14.0, 15.0, 16.0);
    const vec4d r(18.0, 46.0, 74.0, 102.0);
    ASSERT_VEC_EQ(r, m * v);
}

TEST(MatTest, leftMultiplyIdentityMatrixWithVector) {
    const vec4d v(1.0, 2.0, 3.0, 1.0);
    ASSERT_VEC_EQ(v, v * Mat4x4d::Identity);
}

TEST(MatTest, leftMultiplyWithVector) {
    const vec4d v(1.0, 2.0, 3.0, 1.0);
    const Mat4x4d m( 1.0,  2.0,  3.0,  4.0,
                     5.0,  6.0,  7.0,  8.0,
                     9.0, 10.0, 11.0, 12.0,
                    13.0, 14.0, 15.0, 16.0);
    const vec4d r(51.0, 58.0, 65.0, 72.0);
    ASSERT_VEC_EQ(r, v * m);
}

TEST(MatTest, rightMultiplyWithVectorOneLessDimension) {
    const vec3d v(1.0, 2.0, 3.0);
    const Mat4x4d m( 1.0,  2.0,  3.0,  4.0,
                     5.0,  6.0,  7.0,  8.0,
                     9.0, 10.0, 11.0, 12.0,
                    13.0, 14.0, 15.0, 16.0);
    const vec4d r(18.0, 46.0, 74.0, 102.0);
    ASSERT_VEC_EQ(toCartesianCoords(r), m * v);
}

TEST(MatTest, leftMultiplyWithVectorOneLessDimension) {
    const vec3d v(1.0, 2.0, 3.0);
    const Mat4x4d m( 1.0,  2.0,  3.0,  4.0,
                     5.0,  6.0,  7.0,  8.0,
                     9.0, 10.0, 11.0, 12.0,
                    13.0, 14.0, 15.0, 16.0);
    const vec4d r(51.0, 58.0, 65.0, 72.0);
    ASSERT_VEC_EQ(toCartesianCoords(r), v * m);
}

TEST(MatTest, rightMultiplyWithListOfVectors) {
    vec4d::List v;
    v.push_back(vec4d(1.0, 2.0, 3.0, 1.0));
    v.push_back(vec4d(2.0, 3.0, 4.0, 1.0));
    v.push_back(vec4d(3.0, 2.0, 7.0, 23.0));

    const Mat4x4d m( 1.0,  2.0,  3.0,  4.0,
                     5.0,  6.0,  7.0,  8.0,
                     9.0, 10.0, 11.0, 12.0,
                    13.0, 14.0, 15.0, 16.0);

    vec4d::List r;
    r.push_back(vec4d(18.0, 46.0, 74.0, 102.0));
    r.push_back(vec4d(24.0, 64.0, 104.0, 144.0));
    r.push_back(vec4d(120.0, 260.0, 400.0, 540.0));

    const vec4d::List o = m * v;
    for (size_t i = 0; i < 3; i++)
        ASSERT_VEC_EQ(r[i], o[i]);
}

TEST(MatTest, leftMultiplyWithListOfVectors) {
    vec4d::List v;
    v.push_back(vec4d(1.0, 2.0, 3.0, 1.0));
    v.push_back(vec4d(2.0, 3.0, 4.0, 1.0));
    v.push_back(vec4d(3.0, 2.0, 3.0, 23.0));

    const Mat4x4d m( 1.0,  2.0,  3.0,  4.0,
                     5.0,  6.0,  7.0,  8.0,
                     9.0, 10.0, 11.0, 12.0,
                    13.0, 14.0, 15.0, 16.0);

    vec4d::List r;
    r.push_back(vec4d(51.0, 58.0, 65.0, 72.0));
    r.push_back(vec4d(66.0, 76.0, 86.0, 96.0));
    r.push_back(vec4d(339.0, 370.0, 401.0, 432.0));

    const vec4d::List o = v * m;
    for (size_t i = 0; i < 3; i++)
        ASSERT_VEC_EQ(r[i], o[i]);
}

TEST(MatTest, rightMultiplyWithListOfVectorsOneLessDimension) {
    vec3d::List v;
    v.push_back(vec3d(1.0, 2.0, 3.0));
    v.push_back(vec3d(2.0, 3.0, 4.0));
    v.push_back(vec3d(3.0 / 23.0, 2.0 / 23.0, 7.0 / 23.0));

    const Mat4x4d m( 1.0,  2.0,  3.0,  4.0,
                     5.0,  6.0,  7.0,  8.0,
                     9.0, 10.0, 11.0, 12.0,
                    13.0, 14.0, 15.0, 16.0);

    vec3d::List r;
    r.push_back(toCartesianCoords(vec4d(18.0, 46.0, 74.0, 102.0)));
    r.push_back(toCartesianCoords(vec4d(24.0, 64.0, 104.0, 144.0)));
    r.push_back(toCartesianCoords(vec4d(120.0, 260.0, 400.0, 540.0)));

    const vec3d::List o = m * v;
    for (size_t i = 0; i < 3; i++)
        ASSERT_VEC_EQ(r[i], o[i]);
}

TEST(MatTest, leftMultiplyWithListOfVectorsOneLessDimension) {
    vec3d::List v;
    v.push_back(vec4d(1.0, 2.0, 3.0));
    v.push_back(vec4d(2.0, 3.0, 4.0));
    v.push_back(vec4d(3.0 / 23.0, 2.0 / 23.0, 3.0 / 23.0));

    const Mat4x4d m( 1.0,  2.0,  3.0,  4.0,
                     5.0,  6.0,  7.0,  8.0,
                     9.0, 10.0, 11.0, 12.0,
                    13.0, 14.0, 15.0, 16.0);

    vec3d::List r;
    r.push_back(toCartesianCoords(vec4d(51.0, 58.0, 65.0, 72.0)));
    r.push_back(toCartesianCoords(vec4d(66.0, 76.0, 86.0, 96.0)));
    r.push_back(toCartesianCoords(vec4d(339.0, 370.0, 401.0, 432.0)));

    const vec3d::List o = v * m;
    for (size_t i = 0; i < 3; i++)
        ASSERT_VEC_EQ(r[i], o[i]);
}

TEST(MatTest, indexedAccess) {
    const Mat4x4d m( 1.0,  2.0,  3.0,  4.0,
                     5.0,  6.0,  7.0,  8.0,
                     9.0, 10.0, 11.0, 12.0,
                    13.0, 14.0, 15.0, 16.0);

    for (size_t c = 0; c < 4; ++c) {
        for (size_t r = 0; r < 4; ++r) {
            ASSERT_DOUBLE_EQ(m.v[c][r], m[c][r]);
        }
    }
}

TEST(MatTest, equal) {
    const Mat4x4d m( 1.0,  2.0,  3.0,  4.0,
                     5.0,  6.0,  7.0,  8.0,
                     9.0, 10.0, 11.0, 12.0,
                    13.0, 14.0, 15.0, 16.0);
    const Mat4x4d n = m;
    const Mat4x4d o( 2.0,  2.0,  3.0,  4.0,
                     5.0,  8.0,  7.0,  8.0,
                     9.0, 11.0, 11.0, 12.0,
                    13.0, 14.0, 15.0, 16.0);

    ASSERT_TRUE(equal(m, n, 0.0));
    ASSERT_FALSE(equal(m, o, 0.0));
}

TEST(MatTest, null) {
    ASSERT_TRUE(isZero(Mat4x4d::Null));
    ASSERT_FALSE(isZero(Mat4x4d::Identity));
}

TEST(MatTest, fill) {
    ASSERT_MAT_EQ(Mat4x4d::Null, Mat4x4d::fill(0.0));
}

TEST(MatTest, transpose) {
    Mat<double, 4, 4> m;
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

TEST(MatTest, minor) {
    const Mat4x4d m( 1.0,  2.0,  3.0,  4.0,
                     5.0,  6.0,  7.0,  8.0,
                     9.0, 10.0, 11.0, 12.0,
                    13.0, 14.0, 15.0, 16.0);
    const Mat3x3d m00( 6.0,  7.0,  8.0,
                      10.0, 11.0, 12.0,
                      14.0, 15.0, 16.0);
    const Mat3x3d m33( 1.0,  2.0,  3.0,
                       5.0,  6.0,  7.0,
                       9.0, 10.0, 11.0);
    const Mat3x3d m12( 1.0,  2.0,  4.0,
                       9.0, 10.0, 12.0,
                      13.0, 14.0, 16.0);
    const Mat3x3d m21( 1.0,  3.0,  4.0,
                       5.0,  7.0,  8.0,
                      13.0, 15.0, 16.0);
    ASSERT_MAT_EQ(m00, extractMinor(m, 0, 0));
    ASSERT_MAT_EQ(m33, extractMinor(m, 3, 3));
    ASSERT_MAT_EQ(m12, extractMinor(m, 1, 2));
    ASSERT_MAT_EQ(m21, extractMinor(m, 2, 1));
}

TEST(MatTest, determinant) {
    const Mat4x4d m1( 1.0,  2.0,  3.0,  4.0,
                      5.0,  6.0,  7.0,  8.0,
                      9.0, 10.0, 11.0, 12.0,
                     13.0, 14.0, 15.0, 16.0);
    const Mat4x4d m2(65.0, 12.0, -3.0, -5.0,
                     -5.0,  1.0,  0.0,  0.0,
                     19.0, 10.0, 11.0,  8.0,
                      0.0,  1.0, -8.0,  3.0);
    const Mat4x4d m3( 3.0,  2.0, -1.0,  4.0,
                      2.0,  1.0,  5.0,  7.0,
                      0.0,  5.0,  2.0, -6.0,
                     -1.0,  2.0,  1.0,  0.0);
    ASSERT_DOUBLE_EQ(0.0, computeDeterminant(Mat4x4d::Null));
    ASSERT_DOUBLE_EQ(1.0, computeDeterminant(Mat4x4d::Identity));
    ASSERT_DOUBLE_EQ(0.0, computeDeterminant(m1));
    ASSERT_DOUBLE_EQ(15661.0, computeDeterminant(m2));
    ASSERT_DOUBLE_EQ(-418.0, computeDeterminant(m3));
}

TEST(MatTest, adjugate) {
    const Mat4x4d m1( 1.0,  2.0,  3.0,  4.0,
                      5.0,  6.0,  7.0,  8.0,
                      9.0, 10.0, 11.0, 12.0,
                     13.0, 14.0, 15.0, 16.0);
    const Mat4x4d m2(65.0, 12.0, -3.0, -5.0,
                     -5.0,  1.0,  0.0,  0.0,
                     19.0, 10.0, 11.0,  8.0,
                      0.0,  1.0, -8.0,  3.0);
    const Mat4x4d m3( 3.0,  2.0, -1.0,  4.0,
                      2.0,  1.0,  5.0,  7.0,
                      0.0,  5.0,  2.0, -6.0,
                     -1.0,  2.0,  1.0,  0.0);
    const Mat4x4d r1(0.0, 0.0, 0.0, 0.0,
                     0.0, 0.0, 0.0, 0.0,
                     0.0, 0.0, 0.0, 0.0,
                     0.0, 0.0, 0.0, 0.0);
    const Mat4x4d r2(  97.0, -1685.0,  49.0,    31.0,
                      485.0,  7236.0, 245.0,	   155.0,
                     -167.0,	  -651.0, 400.0, -1345.0,
                     -607.0, -4148.0, 985.0,  1582.0);
    const Mat4x4d r3(-47.0, -28.0, -64.0,  221.0,
                     -56.0,  20.0, -14.0, -128.0,
                      65.0, -68.0, -36.0,   59.0,
                     -25.0,  -6.0,  46.0,  -87.0);

    ASSERT_MAT_EQ(Mat4x4d::Identity, computeAdjugate(Mat4x4d::Identity));
    ASSERT_MAT_EQ(Mat4x4d::Null, computeAdjugate(Mat4x4d::Null));
    ASSERT_MAT_EQ(r1, computeAdjugate(m1));
    ASSERT_MAT_EQ(r2, computeAdjugate(m2));
    ASSERT_MAT_EQ(r3, computeAdjugate(m3));
}

template <typename T, size_t S>
void ASSERT_INVERTIBLE(const Mat<T,S,S>& expected, const Mat<T,S,S>& actual) {
    auto [invertible, inverse] = invert(actual);
    ASSERT_MAT_EQ(expected, inverse);
    ASSERT_TRUE(invertible);
}

template <typename T, size_t S>
void ASSERT_NOT_INVERTIBLE(const Mat<T,S,S>& actual) {
    auto [invertible, inverse] = invert(actual);
    ASSERT_FALSE(invertible);
}

TEST(MatTest, invertedMatrix) {
    const Mat4x4d m1( 1.0,  2.0,  3.0,  4.0,
                      5.0,  6.0,  7.0,  8.0,
                      9.0, 10.0, 11.0, 12.0,
                     13.0, 14.0, 15.0, 16.0);
    const Mat4x4d m2(65.0, 12.0, -3.0, -5.0,
                     -5.0,  1.0,  0.0,  0.0,
                     19.0, 10.0, 11.0,  8.0,
                      0.0,  1.0, -8.0,  3.0);
    const Mat4x4d m3( 0.0, -1.0,  0.0,    0.0,
                      0.0,  0.0,  1.0,  128.0,
                     -1.0,  0.0,  0.0,    0.0,
                      0.0,  0.0,  0.0,    1.0);
    const Mat4x4d m4( 0.0,  0.0, -1.0,    0.0,
                     -1.0,  0.0,  0.0,    0.0,
                      0.0,  1.0,  0.0, -128.0,
                      0.0,  0.0,  0.0,    1.0);
    const Mat4x4d r2( 0.0061937296468936, -0.10759210778367, 0.0031287912649256, 0.0019794393716876,
                      0.030968648234468,   0.46203946108167, 0.015643956324628,  0.0098971968584382,
                     -0.01066343145393,   -0.04156822680544, 0.025541153183066, -0.08588212757806,
                     -0.038758699955303,  -0.2648617585084,  0.062895089713301,  0.10101526083903);

    ASSERT_INVERTIBLE(Mat4x4d::Identity, Mat4x4d::Identity);
    ASSERT_INVERTIBLE(r2, m2);
    ASSERT_INVERTIBLE(m4, m3);
    ASSERT_NOT_INVERTIBLE(Mat4x4d::Null);
    ASSERT_NOT_INVERTIBLE(m1);
}

TEST(MatTest, rotationMatrixWithEulerAngles) {
    ASSERT_MAT_EQ(Mat4x4d::Rot90XCCW, rotationMatrix(Math::radians(90.0), 0.0, 0.0));
    ASSERT_MAT_EQ(Mat4x4d::Rot90YCCW, rotationMatrix(0.0, Math::radians(90.0), 0.0));
    ASSERT_MAT_EQ(Mat4x4d::Rot90ZCCW, rotationMatrix(0.0, 0.0, Math::radians(90.0)));
}

TEST(MatTest, rotationMatrixWithAngleAndAxis) {
    ASSERT_MAT_EQ(Mat4x4d::Rot90XCCW, rotationMatrix(vec3d::pos_x, Math::radians(90.0)));
    ASSERT_MAT_EQ(Mat4x4d::Rot90YCCW, rotationMatrix(vec3d::pos_y, Math::radians(90.0)));
    ASSERT_MAT_EQ(Mat4x4d::Rot90ZCCW, rotationMatrix(vec3d::pos_z, Math::radians(90.0)));
    ASSERT_VEC_EQ(vec3d::pos_y, rotationMatrix(vec3d::pos_z, Math::radians(90.0)) * vec3d::pos_x);
}

TEST(MatTest, rotationMatrixWithQuaternion) {
    ASSERT_MAT_EQ(Mat4x4d::Rot90XCCW, rotationMatrix(Quatd(vec3d::pos_x, Math::radians(90.0))));
    ASSERT_MAT_EQ(Mat4x4d::Rot90YCCW, rotationMatrix(Quatd(vec3d::pos_y, Math::radians(90.0))));
    ASSERT_MAT_EQ(Mat4x4d::Rot90ZCCW, rotationMatrix(Quatd(vec3d::pos_z, Math::radians(90.0))));


    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    for (size_t i = 0; i < 10; ++i) {
        vec3d axis;
        for (size_t j = 0; j < 3; ++j)
            axis[j] = (static_cast<double>(std::rand()) / static_cast<double>(RAND_MAX));
        axis = normalize(axis);
        const double angle = (static_cast<double>(std::rand()) / static_cast<double>(RAND_MAX))*2.0*Math::Cd::pi();
        ASSERT_MAT_EQ(rotationMatrix(axis, angle), rotationMatrix(Quatd(axis, angle)));
    }
}

TEST(MatTest, translationMatrix) {
    const vec3d v(2.0, 3.0, 4.0);
    const Mat4x4d t = translationMatrix(v);
    
    ASSERT_VEC_EQ(t[0], vec4d::pos_x);
    ASSERT_VEC_EQ(t[1], vec4d::pos_y);
    ASSERT_VEC_EQ(t[2], vec4d::pos_z);
    ASSERT_VEC_EQ(t[3], vec4d(v, 1.0));
}

TEST(MatTest, scalingMatrix) {
    const vec3d v(2.0, 3.0, 4.0);
    const Mat4x4d t = scalingMatrix(v);
    
    for (size_t c = 0; c < 4; ++c) {
        for (size_t r = 0; r < 4; ++r) {
            if (c == r) {
                if (c < 3)
                    ASSERT_DOUBLE_EQ(v[c], t[c][r]);
                else
                    ASSERT_DOUBLE_EQ(1.0, t[c][r]);
            } else {
                ASSERT_DOUBLE_EQ(0.0, t[c][r]);
            }
        }
    }
}
