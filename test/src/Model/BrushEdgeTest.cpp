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

#include "CollectionUtils.h"
#include "TrenchBroom.h"
#include "VecMath.h"
#include "Model/BrushEdge.h"
#include "Model/BrushFaceGeometry.h"
#include "Model/BrushVertex.h"
#include "TestUtils.h"

namespace TrenchBroom {
    namespace Model {
        TEST(BrushEdgeTest, constructWithStartAndEnd) {
            BrushVertex* start = new BrushVertex(Vec3::Null);
            BrushVertex* end = new BrushVertex(Vec3::Null);
            BrushEdge edge(start, end);
            
            ASSERT_EQ(start, edge.start);
            ASSERT_EQ(end, edge.end);
            
            delete start;
            delete end;
        }
        
        TEST(BrushEdgeTest, mark) {
            BrushVertex* start = new BrushVertex(Vec3(0.0, 0.0, -1.0));
            BrushVertex* end = new BrushVertex(Vec3(0.0, 0.0, 1.0));
            BrushEdge edge(start, end);

            start->updateMark(Plane3(2.0, Vec3::PosZ));
            end->updateMark(Plane3(2.0, Vec3::PosZ));
            edge.updateMark();
            ASSERT_EQ(BrushEdge::Mark_Keep, edge.mark);
            
            start->updateMark(Plane3(1.0, Vec3::PosZ));
            end->updateMark(Plane3(1.0, Vec3::PosZ));
            edge.updateMark();
            ASSERT_EQ(BrushEdge::Mark_Keep, edge.mark);

            start->updateMark(Plane3(0.0, Vec3::PosZ));
            end->updateMark(Plane3(0.0, Vec3::PosZ));
            edge.updateMark();
            ASSERT_EQ(BrushEdge::Mark_Split, edge.mark);
            
            start->updateMark(Plane3(-1.0, Vec3::PosZ));
            end->updateMark(Plane3(-1.0, Vec3::PosZ));
            edge.updateMark();
            ASSERT_EQ(BrushEdge::Mark_Drop, edge.mark);

            start->updateMark(Plane3(-2.0, Vec3::PosZ));
            end->updateMark(Plane3(-2.0, Vec3::PosZ));
            edge.updateMark();
            ASSERT_EQ(BrushEdge::Mark_Drop, edge.mark);

            delete start;
            delete end;
        }
        
        TEST(BrushEdgeTest, split) {
            BrushVertex* start = new BrushVertex(Vec3(3.0, 2.0, 1.0));
            BrushVertex* end = new BrushVertex(Vec3(3.0, 2.0, 10.0));
            BrushEdge edge(start, end);
            
            const Plane3 plane(8.0, Vec3::PosZ);
            start->updateMark(plane);
            end->updateMark(plane);
            edge.updateMark();
            
            BrushVertex* newVertex = edge.split(plane);
            ASSERT_TRUE(newVertex != NULL);
            ASSERT_VEC_EQ(Vec3(3.0, 2.0, 8.0), newVertex->position);
            
            delete start;
            delete end;
            delete newVertex;
        }
        
        TEST(BrushEdgeTest, startAndEndVertexForSide) {
            BrushVertex* start = new BrushVertex(Vec3::Null);
            BrushVertex* end = new BrushVertex(Vec3::Null);
            
            BrushEdge* edge = new BrushEdge(start, end);
            
            BrushFaceGeometry* left = new BrushFaceGeometry();
            BrushFaceGeometry* right = new BrushFaceGeometry();
            BrushFaceGeometry* neither = new BrushFaceGeometry();

            right->addForwardEdge(edge);
            left->addBackwardEdge(edge);
            
            ASSERT_EQ(NULL, edge->startVertex(NULL));
            ASSERT_EQ(NULL, edge->startVertex(neither));
            ASSERT_EQ(start, edge->startVertex(right));
            ASSERT_EQ(end, edge->startVertex(left));
            
            ASSERT_EQ(NULL, edge->endVertex(NULL));
            ASSERT_EQ(NULL, edge->endVertex(neither));
            ASSERT_EQ(end, edge->endVertex(right));
            ASSERT_EQ(start, edge->endVertex(left));
            
            delete left;
            delete right;
            delete neither;
            delete edge;
            delete start;
            delete end;
        }
        
        TEST(BrushEdgeTest, findBrushEdge) {
            BrushVertex* e1s = new BrushVertex(Vec3(1.0, 2.0, 3.0));
            BrushVertex* e1e = new BrushVertex(Vec3(2.0, 3.0, -1.0));
            BrushVertex* e2s = new BrushVertex(Vec3(0.3823, -37373.002, 1231.12312312474));
            BrushVertex* e2e = new BrushVertex(Vec3(483.0, -2343.230, 0.0034));
            
            BrushEdge* e1 = new BrushEdge(e1s, e1e);
            BrushEdge* e2 = new BrushEdge(e2s, e2e);
            
            BrushEdgeList list;
            list.push_back(e1);
            list.push_back(e2);
            
            BrushEdgeList::iterator notFound   = findBrushEdge(list, Vec3(3.0, 2.0, 1.0), Vec3::Null);
            BrushEdgeList::iterator e1Forward  = findBrushEdge(list, e1->start->position, e1->end->position);
            BrushEdgeList::iterator e1Backward = findBrushEdge(list, e1->end->position, e1->start->position);
            BrushEdgeList::iterator e2Forward  = findBrushEdge(list, e2->start->position, e2->end->position);
            BrushEdgeList::iterator e2Backward = findBrushEdge(list, e2->end->position, e2->start->position);
            
            ASSERT_EQ(list.end(), notFound);
            ASSERT_EQ(e1, *e1Forward);
            ASSERT_EQ(e1, *e1Backward);
            ASSERT_EQ(e2, *e2Forward);
            ASSERT_EQ(e2, *e2Backward);
            
            VectorUtils::clearAndDelete(list);
            delete e1s;
            delete e1e;
            delete e2s;
            delete e2e;
        }
    }
}
