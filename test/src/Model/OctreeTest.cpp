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

#include "Exceptions.h"
#include "VecMath.h"
#include "Model/Octree.h"
#include "Model/Object.h"
#include "Model/Brush.h"

namespace TrenchBroom {
    namespace Model {
        TEST(OctreeTest, insertObject) {
            const BBox3f bounds(-128.0f, +128.0f);
            const float minSize = 32.0f;
            Octree<float,int> octree(bounds, minSize);
            
            const int a = 1;
            const BBox3f aBounds(1.0f, 2.0f);
            octree.insert(aBounds, a);
            ASSERT_TRUE(octree.containsObject(aBounds, a));
        }
        
        TEST(OctreeTest, insertTooLargeObject) {
            const BBox3f bounds(-128.0f, +128.0f);
            const float minSize = 32.0f;
            Octree<float,int> octree(bounds, minSize);
            
            const int a = 1;
            const BBox3f aBounds(-129.0f, 2.0f);
            ASSERT_THROW(octree.insert(aBounds, a), OctreeException);
        }
        
        TEST(OctreeTest, removeExistingObject) {
            const BBox3f bounds(-128.0f, +128.0f);
            const float minSize = 32.0f;
            Octree<float,int> octree(bounds, minSize);
            
            const int a = 1;
            const BBox3f aBounds(1.0f, 2.0f);
            octree.insert(aBounds, a);

            ASSERT_TRUE(octree.containsObject(aBounds, a));
            octree.remove(a);
            ASSERT_FALSE(octree.containsObject(aBounds, a));
        }
        
        TEST(OctreeTest, removeNonExistingObject) {
            const BBox3f bounds(-128.0f, +128.0f);
            const float minSize = 32.0f;
            Octree<float,int> octree(bounds, minSize);
            
            const int a = 1;
            const int b = 2;
            const BBox3f aBounds(1.0f, 2.0f);
            octree.insert(aBounds, a);
            ASSERT_THROW(octree.remove(b), OctreeException);
        }
    }
}
