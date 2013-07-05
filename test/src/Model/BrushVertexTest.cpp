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

#include "Model/BrushVertex.h"
#include "VecMath.h"

#include "CollectionUtils.h"
#include "TestUtils.h"

namespace TrenchBroom {
    namespace Model {
        TEST(BrushVertexTest, constructWithPosition) {
            Vec3 position(1.0f, 2.0f, 3.0f);
            BrushVertex vertex(position);
            ASSERT_EQ(position, vertex.position());
        }
        
        TEST(BrushVertexTest, getMark) {
            BrushVertex v(Vec3::Null);
            v.updateMark(Plane3(-1.0, Vec3::PosZ));
            ASSERT_EQ(BrushVertex::Drop, v.mark());
            v.updateMark(Plane3( 0.0, Vec3::PosZ));
            ASSERT_EQ(BrushVertex::Undecided, v.mark());
            v.updateMark(Plane3( 1.0, Vec3::PosZ));
            ASSERT_EQ(BrushVertex::Keep, v.mark());
        }
        
        TEST(BrushVertexTest, findBrushVertex) {
            BrushVertex* v1 = new BrushVertex(Vec3::Null);
            BrushVertex* v2 = new BrushVertex(Vec3(1.3232, 0.3332, -33123.2954));
            BrushVertex* v3 = new BrushVertex(Vec3(1.0, 2.0, 3.0));
            
            BrushVertex::List list;
            list.push_back(v1);
            list.push_back(v2);
            list.push_back(v3);
            
            BrushVertex::List::iterator notFound = findBrushVertex(list, Vec3(-1.0, 1.0, -1.0));
            BrushVertex::List::iterator  v1Found = findBrushVertex(list, v1->position());
            BrushVertex::List::iterator  v2Found = findBrushVertex(list, v2->position());
            BrushVertex::List::iterator  v3Found = findBrushVertex(list, v3->position());
            
            ASSERT_EQ(list.end(), notFound);
            ASSERT_EQ(v1, *v1Found);
            ASSERT_EQ(v2, *v2Found);
            ASSERT_EQ(v3, *v3Found);
            
            VectorUtils::clearAndDelete(list);
        }
    }
}
