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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include <gtest/gtest.h>

#include "TrenchBroom.h"
#include "VecMath.h"

#include "Model/PlanePointFinder.h"
#include "Model/BrushFace.h"

namespace TrenchBroom {
    namespace Model {
        TEST(PlanePointFinderTest, parallelPlane) {
            BrushFace::Points points;
            Plane3 test;
            
            Plane3 xy(12.0, Vec3::PosZ);
            PlanePointFinder::findPoints(xy, points, 0);
            ASSERT_TRUE(setPlanePoints(test, points[0], points[1], points[2]));
            ASSERT_TRUE(test.equals(xy));
            
            Plane3 xz(19.72323, Vec3::PosY);
            PlanePointFinder::findPoints(xz, points, 0);
            ASSERT_TRUE(setPlanePoints(test, points[0], points[1], points[2]));
            ASSERT_TRUE(test.normal.equals(xz.normal));
            ASSERT_TRUE(test.distance == Math::round(xz.distance));
            
            Plane3 yz(1223.127372, Vec3::PosX);
            PlanePointFinder::findPoints(yz, points, 0);
            ASSERT_TRUE(setPlanePoints(test, points[0], points[1], points[2]));
            ASSERT_TRUE(test.normal.equals(yz.normal));
            ASSERT_TRUE(test.distance == Math::round(yz.distance));
        }

        TEST(PlanePointFinderTest, nonParallelPlane) {
            BrushFace::Points points;
            Plane3 plane, test;
            
            plane = Plane3(0.0, Vec3(0.8, 0.0, 1.0).normalized());
            PlanePointFinder::findPoints(plane, points, 0);
            ASSERT_TRUE(setPlanePoints(test, points[0], points[1], points[2]));
            ASSERT_TRUE(test.normal.dot(plane.normal) > 0.99);
            ASSERT_TRUE(std::abs(plane.distance - test.distance) <= 1.0);
            
            plane = Plane3(0.7, Vec3(0.8, 0.0, 1.0).normalized());
            PlanePointFinder::findPoints(plane, points, 0);
            ASSERT_TRUE(setPlanePoints(test, points[0], points[1], points[2]));
            ASSERT_TRUE(test.normal.dot(plane.normal) > 0.99);
            ASSERT_TRUE(std::abs(plane.distance - test.distance) <= 1.0);
            
            plane = Plane3(189.23222, Vec3(0.8, 0.4, 1.0).normalized());
            PlanePointFinder::findPoints(plane, points, 0);
            ASSERT_TRUE(setPlanePoints(test, points[0], points[1], points[2]));
            ASSERT_TRUE(test.normal.dot(plane.normal) > 0.99);
            ASSERT_TRUE(std::abs(plane.distance - test.distance) <= 1.0);
            
            plane = Plane3(72.0, Vec3(0.636535, 0.702198, 0.318969).normalized());
            PlanePointFinder::findPoints(plane, points, 0);
            ASSERT_TRUE(setPlanePoints(test, points[0], points[1], points[2]));
            ASSERT_TRUE(test.normal.dot(plane.normal) > 0.99);
            ASSERT_TRUE(std::abs(plane.distance - test.distance) <= 1.0);
            
            plane = Plane3(72.0, Vec3(0.905819, 0.423666, 0.000290979).normalized());
            PlanePointFinder::findPoints(plane, points, 0);
            ASSERT_TRUE(setPlanePoints(test, points[0], points[1], points[2]));
            ASSERT_TRUE(test.normal.dot(plane.normal) > 0.99);
            ASSERT_TRUE(std::abs(plane.distance - test.distance) <= 1.0);
            
            plane = Plane3(1406.0, Vec3(0.98036, 0.19719, 0.00319336).normalized());
            PlanePointFinder::findPoints(plane, points, 0);
            ASSERT_TRUE(setPlanePoints(test, points[0], points[1], points[2]));
            ASSERT_TRUE(test.normal.dot(plane.normal) > 0.99);
            ASSERT_TRUE(std::abs(plane.distance - test.distance) <= 1.0);
            
            plane = Plane3(635.0, Vec3(0.514331, 0.857591, 0.000837219).normalized());
            PlanePointFinder::findPoints(plane, points, 0);
            ASSERT_TRUE(setPlanePoints(test, points[0], points[1], points[2]));
            ASSERT_TRUE(test.normal.dot(plane.normal) > 0.99);
            ASSERT_TRUE(std::abs(plane.distance - test.distance) <= 1.0);
            
            plane = Plane3(1830.0, Vec3(0.515365, 0.606079, 0.60586).normalized());
            PlanePointFinder::findPoints(plane, points, 0);
            ASSERT_TRUE(setPlanePoints(test, points[0], points[1], points[2]));
            ASSERT_TRUE(test.normal.dot(plane.normal) > 0.99);
            ASSERT_TRUE(std::abs(plane.distance - test.distance) <= 1.0);
            
            plane = Plane3(815.0, Vec3(0.0449349, 0.706393, 0.706393).normalized());
            PlanePointFinder::findPoints(plane, points, 0);
            ASSERT_TRUE(setPlanePoints(test, points[0], points[1], points[2]));
            ASSERT_TRUE(test.normal.dot(plane.normal) > 0.99);
            ASSERT_TRUE(std::abs(plane.distance - test.distance) <= 1.0);
            
            plane = Plane3(1594.0, Vec3(0.994042, 0.086082, 0.0668672).normalized());
            PlanePointFinder::findPoints(plane, points, 0);
            ASSERT_TRUE(setPlanePoints(test, points[0], points[1], points[2]));
            ASSERT_TRUE(test.normal.dot(plane.normal) > 0.99);
            ASSERT_TRUE(Math::lte(std::abs(plane.distance - test.distance), 1.0));
            
            plane = Plane3(224.0, Vec3(0.3185, 0.000606089, 0.947923).normalized());
            PlanePointFinder::findPoints(plane, points, 0);
            ASSERT_TRUE(setPlanePoints(test, points[0], points[1], points[2]));
            ASSERT_TRUE(test.normal.dot(plane.normal) > 0.99);
            ASSERT_TRUE(Math::lte(std::abs(plane.distance - test.distance), 1.0));
            
            plane = Plane3(1706.0, Vec3(0.990495, 0.0042303, 0.137485).normalized());
            PlanePointFinder::findPoints(plane, points, 0);
            ASSERT_TRUE(setPlanePoints(test, points[0], points[1], points[2]));
            ASSERT_TRUE(test.normal.dot(plane.normal) > 0.99);
            ASSERT_TRUE(Math::lte(std::abs(plane.distance - test.distance), 1.0));
            
            plane = Plane3(1861.0, Vec3(835.0, 825.0, 3703.0).normalized());
            PlanePointFinder::findPoints(plane, points, 0);
            ASSERT_TRUE(setPlanePoints(test, points[0], points[1], points[2]));
            ASSERT_TRUE(test.normal.dot(plane.normal) > 0.99);
            ASSERT_TRUE(Math::lte(std::abs(plane.distance - test.distance), 1.0));
            
            plane = Plane3(1630.0, Vec3(625.0, 1418.0, 1418.0).normalized());
            PlanePointFinder::findPoints(plane, points, 0);
            ASSERT_TRUE(setPlanePoints(test, points[0], points[1], points[2]));
            ASSERT_TRUE(test.normal.dot(plane.normal) > 0.99);
            ASSERT_TRUE(Math::lte(std::abs(plane.distance - test.distance), 1.0));
            
            plane = Plane3(442.0, Vec3(1424.0, 2160.0, 2160.0).normalized());
            PlanePointFinder::findPoints(plane, points, 0);
            ASSERT_TRUE(setPlanePoints(test, points[0], points[1], points[2]));
            ASSERT_TRUE(test.normal.dot(plane.normal) > 0.99);
            ASSERT_TRUE(Math::lte(std::abs(plane.distance - test.distance), 1.0));
            
            // planes with a "diagonal" XY normal are the worst!
            plane = Plane3(839.0, Vec3(3998.0, 3998.0, 1948.0).normalized());
            PlanePointFinder::findPoints(plane, points, 0);
            ASSERT_TRUE(setPlanePoints(test, points[0], points[1], points[2]));
            ASSERT_TRUE(test.normal.dot(plane.normal) > 0.99);
            ASSERT_TRUE(Math::lte(std::abs(plane.distance - test.distance), 1.0));
            
            plane = Plane3(1906.0, Vec3(2522.0, 1.0, 1600.0).normalized());
            PlanePointFinder::findPoints(plane, points, 0);
            ASSERT_TRUE(setPlanePoints(test, points[0], points[1], points[2]));
            ASSERT_TRUE(test.normal.dot(plane.normal) > 0.99);
            ASSERT_TRUE(Math::lte(std::abs(plane.distance - test.distance), 1.0));
            
            setPlanePoints(plane, Vec3(160.01, 176.0, 128.0), Vec3(160.01, 176.0, 0.0), Vec3(112.0, 160.0, 128.0));
            PlanePointFinder::findPoints(plane, points, 0);
            ASSERT_TRUE(setPlanePoints(test, points[0], points[1], points[2]));
            ASSERT_TRUE(test.normal.dot(plane.normal) > 0.99);
            ASSERT_TRUE(Math::lte(std::abs(plane.distance - test.distance), 1.0));
        }
    }
}
