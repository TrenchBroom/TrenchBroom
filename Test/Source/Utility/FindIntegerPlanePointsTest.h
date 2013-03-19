/*
 Copyright (C) 2010-2012 Kristian Duske
 
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

#ifndef TrenchBroom_FindIntegerPlanePointsTest_h
#define TrenchBroom_FindIntegerPlanePointsTest_h

#include "TestSuite.h"
#include "Utility/FindIntegerPlanePoints.h"
#include "Utility/VecMath.h"

#include <cassert>
#include <functional>

namespace TrenchBroom {
    namespace Math {
        class FindIntegerPlanePointsTest : public TestSuite<FindIntegerPlanePointsTest> {
        protected:
            void registerTestCases() {
                registerTestCase(&FindIntegerPlanePointsTest::testParallelPlane);
            }
        public:
            void testParallelPlane() {
                Vec3f points[3];
                Plane test;

                Plane xy(Vec3f::PosZ, 12.0f);
                FindIntegerPlanePoints::findPoints(xy, points);
                test.setPoints(points[0], points[1], points[2]);
                assert(test.equals(xy));
                
                Plane xz(Vec3f::PosY, 19.72323f);
                FindIntegerPlanePoints::findPoints(xz, points);
                test.setPoints(points[0], points[1], points[2]);
                assert(test.normal.equals(xz.normal));
                assert(test.distance == Math::round(xz.distance));
            }
        };
    }
}

#endif
