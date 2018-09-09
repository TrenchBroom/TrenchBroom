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
#include <vecmath/vec_decl.h>
#include <vecmath/vec_impl.h>
#include <vecmath/segment_decl.h>
#include <vecmath/segment_impl.h>
#include <vecmath/utils.h>
#include "TestUtils.h"

TEST(SegmentTest, contains1) {
    const auto z = vm::vec3d::zero;
    const auto o = vm::vec3d(1.0, 0.0, 0.0);
    const auto h = vm::vec3d(0.5, 0.0, 0.0);
    const auto n = vm::vec3d(0.5, 1.0, 0.0);

    ASSERT_TRUE( vm::segment3d(z, o).contains(z, vm::Cd::almostZero()));
    ASSERT_TRUE( vm::segment3d(z, o).contains(h, vm::Cd::almostZero()));
    ASSERT_TRUE( vm::segment3d(z, o).contains(o, vm::Cd::almostZero()));
    ASSERT_FALSE(vm::segment3d(z, o).contains(n, vm::Cd::almostZero()));
}

TEST(SegmentTest, contains2) {
    const auto z = vm::vec3d(-64.0, -64.0, 0.0);
    const auto o = vm::vec3d(  0.0, +64.0, 0.0);

    ASSERT_TRUE( vm::segment3d(z, o).contains(z, vm::Cd::almostZero()));
    ASSERT_TRUE( vm::segment3d(z, o).contains(o, vm::Cd::almostZero()));
}
