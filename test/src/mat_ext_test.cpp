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

#include "TrenchBroom.h"
#include <vecmath/mat.h>
#include <vecmath/vec.h>
#include "TestUtils.h"

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
        for (size_t i = 0; i < 3; i++)
        ASSERT_VEC_EQ(r[i], o[i]);
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
        for (size_t i = 0; i < 3; i++)
        ASSERT_VEC_EQ(r[i], o[i]);
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
}
