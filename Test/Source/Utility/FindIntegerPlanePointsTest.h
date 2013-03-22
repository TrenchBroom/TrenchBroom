
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
                std::array<Vec3f, 3> points;
                Plane test;

                Plane xy(Vec3f::PosZ, 12.0f);
                FindIntegerPlanePoints::findPoints(xy, points);
                assert(test.setPoints(points[0], points[1], points[2]));
                assert(test.equals(xy));
                
                Plane xz(Vec3f::PosY, 19.72323f);
                FindIntegerPlanePoints::findPoints(xz, points);
                assert(test.setPoints(points[0], points[1], points[2]));
                assert(test.normal.equals(xz.normal));
                assert(test.distance == Math::round(xz.distance));

                Plane yz(Vec3f::PosY, 1223.127372f);
                FindIntegerPlanePoints::findPoints(yz, points);
                assert(test.setPoints(points[0], points[1], points[2]));
                assert(test.normal.equals(yz.normal));
                assert(test.distance == Math::round(yz.distance));
            }
            
            void testNonParallelPlane() {
                std::array<Vec3f, 3> points;
                Plane plane, test;
                
                plane = Plane(Vec3f(0.8f, 0.0f, 1.0f).normalized(), 0.0f);
                FindIntegerPlanePoints::findPoints(plane, points);
                assert(test.setPoints(points[0], points[1], points[2]));
                assert(test.normal.dot(plane.normal) > 0.99f);
                assert(std::abs(plane.distance - test.distance) <= 1.0f);

                plane = Plane(Vec3f(0.8f, 0.0f, 1.0f).normalized(), 0.7f);
                FindIntegerPlanePoints::findPoints(plane, points);
                assert(test.setPoints(points[0], points[1], points[2]));
                assert(test.normal.dot(plane.normal) > 0.99f);
                assert(std::abs(plane.distance - test.distance) <= 1.0f);
                
                plane = Plane(Vec3f(0.8f, 0.4f, 1.0f).normalized(), 189.23222f);
                FindIntegerPlanePoints::findPoints(plane, points);
                assert(test.setPoints(points[0], points[1], points[2]));
                assert(test.normal.dot(plane.normal) > 0.99f);
                assert(std::abs(plane.distance - test.distance) <= 1.0f);
                
                plane = Plane(Vec3f(0.636535f, 0.702198f, 0.318969f).normalized(), 72.0f);
                FindIntegerPlanePoints::findPoints(plane, points);
                assert(test.setPoints(points[0], points[1], points[2]));
                assert(test.normal.dot(plane.normal) > 0.99f);
                assert(std::abs(plane.distance - test.distance) <= 1.0f);
                
                plane = Plane(Vec3f(0.905819f, 0.423666f, 0.000290979f).normalized(), 72.0f);
                FindIntegerPlanePoints::findPoints(plane, points);
                assert(test.setPoints(points[0], points[1], points[2]));
                assert(test.normal.dot(plane.normal) > 0.99f);
                assert(std::abs(plane.distance - test.distance) <= 1.0f);
                
                plane = Plane(Vec3f(0.98036f, 0.19719f, 0.00319336f).normalized(), 1406.0f);
                FindIntegerPlanePoints::findPoints(plane, points);
                assert(test.setPoints(points[0], points[1], points[2]));
                assert(test.normal.dot(plane.normal) > 0.99f);
                assert(std::abs(plane.distance - test.distance) <= 1.0f);
                
                plane = Plane(Vec3f(0.514331f, 0.857591f, 0.000837219f).normalized(), 635.0f);
                FindIntegerPlanePoints::findPoints(plane, points);
                assert(test.setPoints(points[0], points[1], points[2]));
                assert(test.normal.dot(plane.normal) > 0.99f);
                assert(std::abs(plane.distance - test.distance) <= 1.0f);
                
                plane = Plane(Vec3f(0.515365f, 0.606079f, 0.60586f).normalized(), 1830.0f);
                FindIntegerPlanePoints::findPoints(plane, points);
                assert(test.setPoints(points[0], points[1], points[2]));
                assert(test.normal.dot(plane.normal) > 0.99f);
                assert(std::abs(plane.distance - test.distance) <= 1.0f);
                
                plane = Plane(Vec3f(0.0449349f, 0.706393f, 0.706393f).normalized(), 815.0f);
                FindIntegerPlanePoints::findPoints(plane, points);
                assert(test.setPoints(points[0], points[1], points[2]));
                assert(test.normal.dot(plane.normal) > 0.99f);
                assert(std::abs(plane.distance - test.distance) <= 1.0f);
                
                plane = Plane(Vec3f(0.994042f, 0.086082f, 0.0668672f).normalized(), 1594.0f);
                FindIntegerPlanePoints::findPoints(plane, points);
                assert(test.setPoints(points[0], points[1], points[2]));
                assert(test.normal.dot(plane.normal) > 0.99f);
                assert(Math::lte(std::abs(plane.distance - test.distance), 1.0f));
                
                plane = Plane(Vec3f(0.3185f, 0.000606089f, 0.947923f).normalized(), 224.0f);
                FindIntegerPlanePoints::findPoints(plane, points);
                assert(test.setPoints(points[0], points[1], points[2]));
                assert(test.normal.dot(plane.normal) > 0.99f);
                assert(Math::lte(std::abs(plane.distance - test.distance), 1.0f));
                
                plane = Plane(Vec3f(0.990495f, 0.0042303f, 0.137485f).normalized(), 1706.0f);
                FindIntegerPlanePoints::findPoints(plane, points);
                assert(test.setPoints(points[0], points[1], points[2]));
                assert(test.normal.dot(plane.normal) > 0.99f);
                assert(Math::lte(std::abs(plane.distance - test.distance), 1.0f));
                
                plane = Plane(Vec3f(835.0f, 825.0f, 3703.0f).normalized(), 1861.0f);
                FindIntegerPlanePoints::findPoints(plane, points);
                assert(test.setPoints(points[0], points[1], points[2]));
                assert(test.normal.dot(plane.normal) > 0.99f);
                assert(Math::lte(std::abs(plane.distance - test.distance), 1.0f));
                
                plane = Plane(Vec3f(625.0f, 1418.0f, 1418.0f).normalized(), 1630.0f);
                FindIntegerPlanePoints::findPoints(plane, points);
                assert(test.setPoints(points[0], points[1], points[2]));
                assert(test.normal.dot(plane.normal) > 0.99f);
                assert(Math::lte(std::abs(plane.distance - test.distance), 1.0f));
                
                plane = Plane(Vec3f(1424.0f, 2160.0f, 2160.0f).normalized(), 442.0f);
                FindIntegerPlanePoints::findPoints(plane, points);
                assert(test.setPoints(points[0], points[1], points[2]));
                assert(test.normal.dot(plane.normal) > 0.99f);
                assert(Math::lte(std::abs(plane.distance - test.distance), 1.0f));
                
                // planes with a "diagonal" XY normal are the worst!
                plane = Plane(Vec3f(3998.0f, 3998.0f, 1948.0f).normalized(), 839.0f);
                FindIntegerPlanePoints::findPoints(plane, points);
                assert(test.setPoints(points[0], points[1], points[2]));
                assert(test.normal.dot(plane.normal) > 0.99f);
                assert(Math::lte(std::abs(plane.distance - test.distance), 1.0f));

                plane = Plane(Vec3f(2522.0f, 1.0f, 1600.0f).normalized(), 1906.0f);
                FindIntegerPlanePoints::findPoints(plane, points);
                assert(test.setPoints(points[0], points[1], points[2]));
                assert(test.normal.dot(plane.normal) > 0.99f);
                assert(Math::lte(std::abs(plane.distance - test.distance), 1.0f));
            }
            
            void testRandomPlanes() {
                std::array<Vec3f, 3> points;
                Plane plane, test;

                std::srand(static_cast<unsigned int>(std::time(NULL)));
                for (size_t i = 0; i < 100000; i++) {
                    float x = std::rand() % 4096;
                    float y = std::rand() % 4096;
                    float z = std::rand() % 4096;
                    float d = std::rand() % 2096;
                    
                    plane = Plane(Vec3f(x, y, z).normalized(), d);
                    FindIntegerPlanePoints::findPoints(plane, points);
                    assert(test.setPoints(points[0], points[1], points[2]));
                    assert(test.normal.dot(plane.normal) > 0.99f);
                    assert(Math::lte(std::abs(plane.distance - test.distance), 2.0f));
                }
            }
        };
    }
}

#endif
