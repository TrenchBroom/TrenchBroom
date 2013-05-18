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

#include "TrenchBroom.h"
#include "VecMath.h"
#include "CollectionUtils.h"
#include "Model/BrushEdge.h"
#include "Model/BrushFaceGeometry.h"
#include "Model/BrushVertex.h"

#include <algorithm>

namespace TrenchBroom {
    namespace Model {
        TEST(BrushFaceGeometryTest, HasVertexPositions) {
            const Vec3 p1(1.0, 2.0, 3.0);
            const Vec3 p2(2.0, 3.0, 4.0);
            const Vec3 p3(3.0, 4.0, 5.0);
            const Vec3 p4(4.0, 5.0, 6.0);
            const Vec3 p5(5.0, 6.0, 7.0);

            BrushVertex* v1 = new BrushVertex(p1);
            BrushVertex* v2 = new BrushVertex(p2);
            BrushVertex* v3 = new BrushVertex(p3);
            BrushVertex* v4 = new BrushVertex(p4);
            BrushVertex* v5 = new BrushVertex(p5);
            
            BrushVertexList vertices;
            vertices.push_back(v1);
            vertices.push_back(v2);
            vertices.push_back(v3);
            vertices.push_back(v4);
            vertices.push_back(v5);
            
            BrushEdgeList edges;
            edges.push_back(new BrushEdge(v1, v2));
            edges.push_back(new BrushEdge(v2, v3));
            edges.push_back(new BrushEdge(v3, v4));
            edges.push_back(new BrushEdge(v4, v5));
            edges.push_back(new BrushEdge(v5, v1));
            
            BrushFaceGeometry face(edges);
            
            Vec3::List list;
            list.push_back(p1);
            list.push_back(p2);
            list.push_back(p3);
            list.push_back(p4);
            list.push_back(p5);

            ASSERT_TRUE(face.hasVertexPositions(list));
            for (size_t i = 0; i < list.size(); i++) {
                std::rotate(list.begin(), list.begin() + 1, list.end());
                ASSERT_TRUE(face.hasVertexPositions(list));
            }
            
            Vec3::List reversed = list;
            std::reverse(reversed.begin(), reversed.end());
            ASSERT_FALSE(face.hasVertexPositions(reversed));
            
            Vec3::List swapped = list;
            std::swap(swapped.front(), swapped.back());
            ASSERT_FALSE(face.hasVertexPositions(swapped));
            
            Vec3::List shorter = list;
            shorter.pop_back();
            ASSERT_FALSE(face.hasVertexPositions(shorter));
            
            Vec3::List longer = list;
            list.push_back(list.back());
            ASSERT_FALSE(face.hasVertexPositions(shorter));
            
            VectorUtils::clearAndDelete(edges);
            VectorUtils::clearAndDelete(vertices);
        }
        
        TEST(BrushFaceGeometryTest, FindBrushFaceGeometry) {
            const Vec3 p1(1.0, 2.0, 3.0);
            const Vec3 p2(2.0, 3.0, 4.0);
            const Vec3 p3(3.0, 4.0, 5.0);
            const Vec3 p4(4.0, 5.0, 6.0);
            const Vec3 p5(5.0, 6.0, 7.0);
            
            BrushVertex* v1 = new BrushVertex(p1);
            BrushVertex* v2 = new BrushVertex(p2);
            BrushVertex* v3 = new BrushVertex(p3);
            BrushVertex* v4 = new BrushVertex(p4);
            BrushVertex* v5 = new BrushVertex(p5);
            
            BrushVertexList vertices;
            vertices.push_back(v1);
            vertices.push_back(v2);
            vertices.push_back(v3);
            vertices.push_back(v4);
            vertices.push_back(v5);
            
            BrushEdgeList edges1;
            edges1.push_back(new BrushEdge(v1, v2));
            edges1.push_back(new BrushEdge(v2, v3));
            edges1.push_back(new BrushEdge(v3, v4));
            edges1.push_back(new BrushEdge(v4, v5));
            edges1.push_back(new BrushEdge(v5, v1));

            BrushEdgeList edges2;
            edges2.push_back(new BrushEdge(v1, v2));
            edges2.push_back(new BrushEdge(v2, v3));
            edges2.push_back(new BrushEdge(v3, v1));

            BrushEdgeList edges3;
            edges3.push_back(new BrushEdge(v3, v4));
            edges3.push_back(new BrushEdge(v4, v5));
            edges3.push_back(new BrushEdge(v5, v3));
            
            BrushFaceGeometry* faceGeometry1 = new BrushFaceGeometry(edges1);
            BrushFaceGeometry* faceGeometry2 = new BrushFaceGeometry(edges2);
            BrushFaceGeometry* faceGeometry3 = new BrushFaceGeometry(edges3);
            
            BrushFaceGeometryList faceGeometries;
            faceGeometries.push_back(faceGeometry1);
            faceGeometries.push_back(faceGeometry2);
            faceGeometries.push_back(faceGeometry3);
            
            Vec3::List positions1;
            positions1.push_back(p1);
            positions1.push_back(p2);
            positions1.push_back(p3);
            positions1.push_back(p4);
            positions1.push_back(p5);
            
            Vec3::List positions2;
            positions2.push_back(p1);
            positions2.push_back(p2);
            positions2.push_back(p3);
            
            Vec3::List positions3;
            positions3.push_back(p3);
            positions3.push_back(p4);
            positions3.push_back(p5);
            
            Vec3::List positions4;
            positions4.push_back(p1);
            positions4.push_back(p3);
            positions4.push_back(p5);
            
            for (size_t i = 0; i < positions1.size(); i++) {
                VectorUtils::shift(positions1, i);
                VectorUtils::shift(positions2, i);
                VectorUtils::shift(positions3, i);
                VectorUtils::shift(positions4, i);

                ASSERT_EQ(faceGeometries.begin(), findBrushFaceGeometry(faceGeometries, positions1));
                ASSERT_EQ(faceGeometries.begin() + 1, findBrushFaceGeometry(faceGeometries, positions2));
                ASSERT_EQ(faceGeometries.begin() + 2, findBrushFaceGeometry(faceGeometries, positions3));
                ASSERT_EQ(faceGeometries.end(), findBrushFaceGeometry(faceGeometries, positions4));
            }
            
            VectorUtils::clearAndDelete(faceGeometries);
            VectorUtils::clearAndDelete(edges1);
            VectorUtils::clearAndDelete(edges2);
            VectorUtils::clearAndDelete(edges3);
            VectorUtils::clearAndDelete(vertices);
        }
    }
}
