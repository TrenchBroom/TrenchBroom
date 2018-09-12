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

#include <vecmath/constants.h>
#include <vecmath/vec.h>
#include <vecmath/segment.h>
#include <vecmath/scalar.h>
#include "TestUtils.h"

// TODO 2201: write more tests

namespace vm {
    TEST(SegmentTest, contains1) {
        const auto z = vec3d::zero;
        const auto o = vec3d(1.0, 0.0, 0.0);
        const auto h = vec3d(0.5, 0.0, 0.0);
        const auto n = vec3d(0.5, 1.0, 0.0);
    
        ASSERT_TRUE( segment3d(z, o).contains(z, Cd::almostZero()));
        ASSERT_TRUE( segment3d(z, o).contains(h, Cd::almostZero()));
        ASSERT_TRUE( segment3d(z, o).contains(o, Cd::almostZero()));
        ASSERT_FALSE(segment3d(z, o).contains(n, Cd::almostZero()));
    }
    
    TEST(SegmentTest, contains2) {
        const auto z = vec3d(-64.0, -64.0, 0.0);
        const auto o = vec3d(  0.0, +64.0, 0.0);
    
        ASSERT_TRUE( segment3d(z, o).contains(z, Cd::almostZero()));
        ASSERT_TRUE( segment3d(z, o).contains(o, Cd::almostZero()));
    }
}
