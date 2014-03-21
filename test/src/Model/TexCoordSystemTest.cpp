/*
 Copyright (C) 2010-2014 Kristian Duske
 
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
#include "VecMath.h"
#include "TestUtils.h"

#include "Model/TexCoordSystem.h"
#include "Model/ParallelTexCoordSystem.h"
#include "Model/ParaxialTexCoordSystem.h"

namespace TrenchBroom {
    namespace Model {
        TEST(TexCoordSystemTest, testTransformToParaxial) {
            ParaxialTexCoordSystem system(Vec3::Null, Vec3::PosY, Vec3::PosX);
            
            ASSERT_VEC_EQ(Vec3::Null, system.transformTo(Vec3::Null));
            ASSERT_VEC_EQ(Vec3::Null, system.transformTo(Vec3::Null, Vec2f::Null, Vec2f(2.0f, 2.0f)));
            ASSERT_VEC_EQ(Vec3(2.0, 3.0, 0.0), system.transformTo(Vec3::Null, Vec2f(2.0f, 3.0f)));
            ASSERT_VEC_EQ(Vec3(0.5, -2.0, -1.0), system.transformTo(Vec3(1.0, 1.0, 1.0), Vec2f::Null, Vec2f(2.0f, 0.5f)));
        }
    }
}
