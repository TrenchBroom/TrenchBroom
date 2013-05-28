/*
 Copyright (C) 2010-2013 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <gtest/gtest.h>

#include "Model/BrushFace.h"
#include "Model/BrushFaceTypes.h"
#include "TrenchBroom.h"
#include "Exceptions.h"
#include "VecMath.h"
#include "TestUtils.h"

namespace TrenchBroom {
    namespace Model {
        TEST(BrushFaceTest, ConstructWithValidPoints) {
            const Vec3 point0(0.0,  0.0, 4.0);
            const Vec3 point1(1.f,  0.0, 4.0);
            const Vec3 point2(0.0, -1.0, 4.0);
            
            BrushFacePtr face = BrushFace::newBrushFace(point0, point1, point2);
            ASSERT_VEC_EQ(point0, face->points()[0]);
            ASSERT_VEC_EQ(point1, face->points()[1]);
            ASSERT_VEC_EQ(point2, face->points()[2]);
            ASSERT_VEC_EQ(Vec3::PosZ, face->boundary().normal);
            ASSERT_EQ(4.0, face->boundary().distance);
        }
        
        TEST(BrushFaceTest, ConstructWithColinearPoints) {
            const Vec3 point0(0.0, 0.0, 4.0);
            const Vec3 point1(1.f, 0.0, 4.0);
            const Vec3 point2(2.0, 0.0, 4.0);
            
            ASSERT_THROW(BrushFace::newBrushFace(point0, point1, point2), GeometryException);
        }
    }
}
