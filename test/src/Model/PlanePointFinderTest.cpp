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

#include "Vec.h"
#include "Plane.h"
#include "MathUtils.h"
#include "TestUtils.h"
#include "Model/PlanePointFinder.h"


/* see https://github.com/kduske/TrenchBroom/issues/1033
 commented out because it breaks the release build process
 */
TEST(PlaneTest, planePointFinder) {
    Plane3 plane;
    const Vec3 points[3] = {Vec3(48, 16, 28), Vec3(16.0, 16.0, 27.9980487823486328125), Vec3(48, 18, 22)};
    ASSERT_FALSE(points[1].isInteger());
    ASSERT_TRUE(setPlanePoints(plane, points[0], points[1], points[2]));
    
    
    // Some verts that should lie (very close to) on the plane
    std::vector<Vec3> verts;
    verts.push_back(Vec3(48, 18, 22));
    verts.push_back(Vec3(48, 16, 28));
    verts.push_back(Vec3(16, 16, 28));
    verts.push_back(Vec3(16, 18, 22));
    
    for (size_t i=0; i<verts.size(); i++) {
        FloatType dist = Math::abs(plane.pointDistance(verts[i]));
        ASSERT_LT(dist, 0.01);
    }
    
    // Now find a similar plane with integer points
    
    Vec3 intpoints[3];
    for (size_t i=0; i<3; i++)
        intpoints[i] = points[i];
    
    TrenchBroom::Model::PlanePointFinder::findPoints(plane, intpoints, 3);
    
    ASSERT_TRUE(intpoints[0].isInteger());
    ASSERT_TRUE(intpoints[1].isInteger());
    ASSERT_TRUE(intpoints[2].isInteger());
    
    Plane3 intplane;
    ASSERT_TRUE(setPlanePoints(intplane, intpoints[0], intpoints[1], intpoints[2]));
    //	ASSERT_FALSE(intplane.equals(plane)); no longer fails
    
    // Check that the verts are still close to the new integer plane
    
    for (size_t i=0; i<verts.size(); i++) {
        FloatType dist = Math::abs(intplane.pointDistance(verts[i]));
        ASSERT_LT(dist, 0.01);
    }
}
