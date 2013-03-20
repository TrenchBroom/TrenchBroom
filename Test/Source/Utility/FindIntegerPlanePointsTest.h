
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
#include <cstdlib>
#include <functional>
#include <ctime>

namespace TrenchBroom {
    namespace Math {
        class FindIntegerPlanePointsTest : public TestSuite<FindIntegerPlanePointsTest> {
        protected:
            void registerTestCases() {
                registerTestCase(&FindIntegerPlanePointsTest::testParallelPlane);
                registerTestCase(&FindIntegerPlanePointsTest::testNonParallelPlane);
                registerTestCase(&FindIntegerPlanePointsTest::testRandomPlanes);
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

                Plane yz(Vec3f::PosY, 1223.127372f);
                FindIntegerPlanePoints::findPoints(yz, points);
                test.setPoints(points[0], points[1], points[2]);
                assert(test.normal.equals(yz.normal));
                assert(test.distance == Math::round(yz.distance));
            }
            
            void testNonParallelPlane() {
                Vec3f points[3];
                Plane plane, test;
                
                plane = Plane(Vec3f(0.8f, 0.0f, 1.0f).normalized(), 0.0f);
                FindIntegerPlanePoints::findPoints(plane, points);
                test.setPoints(points[0], points[1], points[2]);
                assert(test.normal.equals(plane.normal));
                assert(test.distance == Math::round(plane.distance));

                plane = Plane(Vec3f(0.8f, 0.0f, 1.0f).normalized(), 0.7f);
                FindIntegerPlanePoints::findPoints(plane, points);
                test.setPoints(points[0], points[1], points[2]);
                assert(test.normal.equals(plane.normal));
                assert(std::abs(plane.distance - test.distance) <= 1.0f);
                
                plane = Plane(Vec3f(0.8f, 0.4f, 1.0f).normalized(), 189.23222f);
                FindIntegerPlanePoints::findPoints(plane, points);
                test.setPoints(points[0], points[1], points[2]);
                assert(test.normal.equals(plane.normal));
                assert(std::abs(plane.distance - test.distance) <= 1.0f);
            }
            
            void testRandomPlanes() {
                Vec3f points[3];
                Plane plane, test;

                std::srand(static_cast<unsigned int>(std::time(NULL)));
                for (size_t i = 0; i < 100; i++) {
                    float x = std::rand() % 4096;
                    float y = std::rand() % 4096;
                    float z = std::rand() % 4096;
                    float d = std::rand() % 2096;
                    
                    plane = Plane(Vec3f(x, y, z).normalized(), d);
                    FindIntegerPlanePoints::findPoints(plane, points);
                    test.setPoints(points[0], points[1], points[2]);
                    assert(test.normal.equals(plane.normal, 0.01f));
                    assert(std::abs(plane.distance - test.distance) <= 1.0f);
                }
            }
        };
    }
}

#endif
