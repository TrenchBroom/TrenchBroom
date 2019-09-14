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

#include <vecmath/forward.h>
#include <vecmath/mat.h>
#include <vecmath/mat_ext.h>
#include <vecmath/vec.h>

#include "test_utils.h"

namespace vm {
    TEST(MatTest, rightMultiplyWithListOfVectors) {
        std::vector<vec4d> v;
        v.push_back(vec4d(1.0, 2.0, 3.0, 1.0));
        v.push_back(vec4d(2.0, 3.0, 4.0, 1.0));
        v.push_back(vec4d(3.0, 2.0, 7.0, 23.0));

        const mat4x4d m( 1.0,  2.0,  3.0,  4.0,
                         5.0,  6.0,  7.0,  8.0,
                         9.0, 10.0, 11.0, 12.0,
                         13.0, 14.0, 15.0, 16.0);

        std::vector<vec4d> r;
        r.push_back(vec4d(18.0, 46.0, 74.0, 102.0));
        r.push_back(vec4d(24.0, 64.0, 104.0, 144.0));
        r.push_back(vec4d(120.0, 260.0, 400.0, 540.0));

        const std::vector<vec4d> o = m * v;
        for (size_t i = 0; i < 3; i++) {
            ASSERT_VEC_EQ(r[i], o[i]);
        }
    }

    TEST(MatTest, leftMultiplyWithListOfVectors) {
        std::vector<vec4d> v;
        v.push_back(vec4d(1.0, 2.0, 3.0, 1.0));
        v.push_back(vec4d(2.0, 3.0, 4.0, 1.0));
        v.push_back(vec4d(3.0, 2.0, 3.0, 23.0));

        const mat4x4d m( 1.0,  2.0,  3.0,  4.0,
                         5.0,  6.0,  7.0,  8.0,
                         9.0, 10.0, 11.0, 12.0,
                         13.0, 14.0, 15.0, 16.0);

        std::vector<vec4d> r;
        r.push_back(vec4d(51.0, 58.0, 65.0, 72.0));
        r.push_back(vec4d(66.0, 76.0, 86.0, 96.0));
        r.push_back(vec4d(339.0, 370.0, 401.0, 432.0));

        const std::vector<vec4d> o = v * m;
        for (size_t i = 0; i < 3; i++) {
            ASSERT_VEC_EQ(r[i], o[i]);
        }
    }

    TEST(MatTest, rightMultiplyWithListOfVectorsOneLessDimension) {
        std::vector<vec3d> v;
        v.push_back(vec3d(1.0, 2.0, 3.0));
        v.push_back(vec3d(2.0, 3.0, 4.0));
        v.push_back(vec3d(3.0 / 23.0, 2.0 / 23.0, 7.0 / 23.0));

        const mat4x4d m( 1.0,  2.0,  3.0,  4.0,
                         5.0,  6.0,  7.0,  8.0,
                         9.0, 10.0, 11.0, 12.0,
                         13.0, 14.0, 15.0, 16.0);

        std::vector<vec3d> r;
        r.push_back(toCartesianCoords(vec4d(18.0, 46.0, 74.0, 102.0)));
        r.push_back(toCartesianCoords(vec4d(24.0, 64.0, 104.0, 144.0)));
        r.push_back(toCartesianCoords(vec4d(120.0, 260.0, 400.0, 540.0)));

        const std::vector<vec3d> o = m * v;
        for (size_t i = 0; i < 3; i++) {
            ASSERT_VEC_EQ(r[i], o[i]);
        }
    }

    TEST(MatTest, leftMultiplyWithListOfVectorsOneLessDimension) {
        std::vector<vec3d> v;
        v.push_back(vec3d(1.0, 2.0, 3.0));
        v.push_back(vec3d(2.0, 3.0, 4.0));
        v.push_back(vec3d(3.0 / 23.0, 2.0 / 23.0, 3.0 / 23.0));

        const mat4x4d m( 1.0,  2.0,  3.0,  4.0,
                         5.0,  6.0,  7.0,  8.0,
                         9.0, 10.0, 11.0, 12.0,
                         13.0, 14.0, 15.0, 16.0);

        std::vector<vec3d> r;
        r.push_back(toCartesianCoords(vec4d(51.0, 58.0, 65.0, 72.0)));
        r.push_back(toCartesianCoords(vec4d(66.0, 76.0, 86.0, 96.0)));
        r.push_back(toCartesianCoords(vec4d(339.0, 370.0, 401.0, 432.0)));

        const std::vector<vec3d> o = v * m;
        for (size_t i = 0; i < 3; i++) {
           ASSERT_VEC_EQ(r[i], o[i]);
        }
    }

    TEST(MatTest, rotationMatrixWithEulerAngles) {
        ASSERT_MAT_EQ(mat4x4d::rot_90_x_ccw, rotationMatrix(toRadians(90.0), 0.0, 0.0));
        ASSERT_MAT_EQ(mat4x4d::rot_90_y_ccw, rotationMatrix(0.0, toRadians(90.0), 0.0));
        ASSERT_MAT_EQ(mat4x4d::rot_90_z_ccw, rotationMatrix(0.0, 0.0, toRadians(90.0)));
    }

    TEST(MatTest, rotationMatrixToEulerAngles_90DegreeRotations) {
        ASSERT_VEC_EQ(vec3d(toRadians(90.0), 0.0, 0.0), rotationMatrixToEulerAngles(mat4x4d::rot_90_x_ccw));
        ASSERT_VEC_EQ(vec3d(0.0, toRadians(90.0), 0.0), rotationMatrixToEulerAngles(mat4x4d::rot_90_y_ccw));
        ASSERT_VEC_EQ(vec3d(0.0, 0.0, toRadians(90.0)), rotationMatrixToEulerAngles(mat4x4d::rot_90_z_ccw));
    }

    TEST(MatTest, rotationMatrixToEulerAngles) {
        const auto roll = toRadians(12.0);
        const auto pitch = toRadians(13.0);
        const auto yaw = toRadians(14.0);

        const auto rotMat = rotationMatrix(roll, pitch, yaw);
        const auto rollPitchYaw = rotationMatrixToEulerAngles(rotMat);

        EXPECT_DOUBLE_EQ(roll, rollPitchYaw.x());
        EXPECT_DOUBLE_EQ(pitch, rollPitchYaw.y());
        EXPECT_DOUBLE_EQ(yaw, rollPitchYaw.z());
    }

    TEST(MatTest, rotationMatrixWithAngleAndAxis) {
        ASSERT_MAT_EQ(mat4x4d::rot_90_x_ccw, rotationMatrix(vec3d::pos_x, toRadians(90.0)));
        ASSERT_MAT_EQ(mat4x4d::rot_90_y_ccw, rotationMatrix(vec3d::pos_y, toRadians(90.0)));
        ASSERT_MAT_EQ(mat4x4d::rot_90_z_ccw, rotationMatrix(vec3d::pos_z, toRadians(90.0)));
        ASSERT_VEC_EQ(vec3d::pos_y, rotationMatrix(vec3d::pos_z, toRadians(90.0)) * vec3d::pos_x);
    }

    TEST(MatTest, rotationMatrixWithQuaternion) {
        ASSERT_MAT_EQ(mat4x4d::rot_90_x_ccw, rotationMatrix(quatd(vec3d::pos_x, toRadians(90.0))));
        ASSERT_MAT_EQ(mat4x4d::rot_90_y_ccw, rotationMatrix(quatd(vec3d::pos_y, toRadians(90.0))));
        ASSERT_MAT_EQ(mat4x4d::rot_90_z_ccw, rotationMatrix(quatd(vec3d::pos_z, toRadians(90.0))));

        std::srand(static_cast<unsigned int>(std::time(nullptr)));
        for (size_t i = 0; i < 10; ++i) {
            vec3d axis;
            for (size_t j = 0; j < 3; ++j) {
                axis[j] = (static_cast<double>(std::rand()) / static_cast<double>(RAND_MAX));
            }
            axis = normalize(axis);
            const double angle = (static_cast<double>(std::rand()) / static_cast<double>(RAND_MAX))*2.0*Cd::pi();
            ASSERT_MAT_EQ(rotationMatrix(axis, angle), rotationMatrix(quatd(axis, angle)));
        }
    }

    TEST(MatTest, translationMatrix) {
        const vec3d v(2.0, 3.0, 4.0);
        const mat4x4d t = translationMatrix(v);

        ASSERT_VEC_EQ(t[0], vec4d::pos_x);
        ASSERT_VEC_EQ(t[1], vec4d::pos_y);
        ASSERT_VEC_EQ(t[2], vec4d::pos_z);
        ASSERT_VEC_EQ(t[3], vec4d(v, 1.0));
    }

    TEST(MatTest, scalingMatrix) {
        const vec3d v(2.0, 3.0, 4.0);
        const mat4x4d t = scalingMatrix(v);

        for (size_t c = 0; c < 4; ++c) {
            for (size_t r = 0; r < 4; ++r) {
                if (c == r) {
                    if (c < 3) {
                        ASSERT_DOUBLE_EQ(v[c], t[c][r]);
                    } else {
                        ASSERT_DOUBLE_EQ(1.0, t[c][r]);
                    }
                } else {
                    ASSERT_DOUBLE_EQ(0.0, t[c][r]);
                }
            }
        }
    }

    TEST(MatTest, mirrorMatrix) {
        const auto mirX = mirrorMatrix<double>(axis::x);
        const auto mirY = mirrorMatrix<double>(axis::y);
        const auto mirZ = mirrorMatrix<double>(axis::z);

        ASSERT_EQ(vec3d::neg_x, mirX * vec3d::pos_x);
        ASSERT_EQ(vec3d::pos_y, mirX * vec3d::pos_y);
        ASSERT_EQ(vec3d::pos_z, mirX * vec3d::pos_z);

        ASSERT_EQ(vec3d::pos_x, mirY * vec3d::pos_x);
        ASSERT_EQ(vec3d::neg_y, mirY * vec3d::pos_y);
        ASSERT_EQ(vec3d::pos_z, mirY * vec3d::pos_z);

        ASSERT_EQ(vec3d::pos_x, mirZ * vec3d::pos_x);
        ASSERT_EQ(vec3d::pos_y, mirZ * vec3d::pos_y);
        ASSERT_EQ(vec3d::neg_z, mirZ * vec3d::pos_z);
    }

    TEST(MatTest, coordinateSystemMatrix) {
        const auto m = coordinateSystemMatrix(vec3d::neg_x, vec3d::neg_y, vec3d::neg_z, vec3d::one);
        ASSERT_EQ(vec3d::neg_x + vec3d::one, m * vec3d::pos_x);
        ASSERT_EQ(vec3d::neg_y + vec3d::one, m * vec3d::pos_y);
        ASSERT_EQ(vec3d::neg_z + vec3d::one, m * vec3d::pos_z);
    }

    TEST(MatTest, planeProjectionMatrix) {
        // I really don't know how to write a test for this right now.
    }

    TEST(MatTest, shearMatrix) {
        ASSERT_EQ(vec3d(1, 1, 1), shearMatrix(0.0, 0.0, 0.0, 0.0, 1.0, 1.0) * vec3d::pos_z);
        ASSERT_EQ(vec3d(0, 0, 0), shearMatrix(0.0, 0.0, 0.0, 0.0, 1.0, 1.0) * vec3d::zero);
        ASSERT_EQ(vec3d(1, 1, 1), shearMatrix(0.0, 0.0, 1.0, 1.0, 0.0, 0.0) * vec3d::pos_y);
        ASSERT_EQ(vec3d(0, 0, 0), shearMatrix(0.0, 0.0, 1.0, 1.0, 0.0, 0.0) * vec3d::zero);
        ASSERT_EQ(vec3d(1, 1, 1), shearMatrix(1.0, 1.0, 0.0, 0.0, 0.0, 0.0) * vec3d::pos_x);
        ASSERT_EQ(vec3d(0, 0, 0), shearMatrix(1.0, 1.0, 0.0, 0.0, 0.0, 0.0) * vec3d::zero);
    }
}
