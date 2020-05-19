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

#include <catch2/catch.hpp>

#include "GTestCompat.h"

#include <vecmath/vec.h>
#include <vecmath/plane.h>
#include <vecmath/scalar.h>
#include "TestUtils.h"
#include "Model/PlanePointFinder.h"

#include <array>

/* see https://github.com/kduske/TrenchBroom/issues/1033
 commented out because it breaks the release build process
 */
TEST_CASE("PlaneTest.planePointFinder", "[PlaneTest]") {
    const vm::vec3 points[3] = {vm::vec3(48, 16, 28), vm::vec3(16.0, 16.0, 27.9980487823486328125), vm::vec3(48, 18, 22)};
    ASSERT_FALSE(vm::is_integral(points[1]));

    auto [valid, plane] = vm::from_points(points[0], points[1], points[2]);
    ASSERT_TRUE(valid);

    // Some verts that should lie (very close to) on the plane
    std::vector<vm::vec3> verts;
    verts.push_back(vm::vec3(48, 18, 22));
    verts.push_back(vm::vec3(48, 16, 28));
    verts.push_back(vm::vec3(16, 16, 28));
    verts.push_back(vm::vec3(16, 18, 22));

    for (size_t i=0; i<verts.size(); i++) {
        FloatType dist = vm::abs(plane.point_distance(verts[i]));
        ASSERT_LT(dist, 0.01);
    }

    // Now find a similar plane with integer points

    std::array<vm::vec3, 3u> intpoints;
    for (size_t i=0; i<3; i++)
        intpoints[i] = points[i];

    TrenchBroom::Model::PlanePointFinder::findPoints(plane, intpoints, 3);

    ASSERT_TRUE(vm::is_integral(intpoints[0], 0.001));
    ASSERT_TRUE(vm::is_integral(intpoints[1], 0.001));
    ASSERT_TRUE(vm::is_integral(intpoints[2], 0.001));

    std::tie(valid, plane) = vm::from_points(intpoints[0], intpoints[1], intpoints[2]);

    // Check that the verts are still close to the new integer plane
    for (size_t i=0; i<verts.size(); i++) {
        FloatType dist = vm::abs(plane.point_distance(verts[i]));
        ASSERT_LT(dist, 0.01);
    }
}
