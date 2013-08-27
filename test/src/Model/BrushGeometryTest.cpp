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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include <gtest/gtest.h>

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Model/BrushGeometry.h"
#include "Model/BrushFace.h"
#include "Model/BrushFaceTypes.h"
#include "Model/BrushVertex.h"
#include "Model/BrushEdge.h"
#include "Model/BrushFaceGeometry.h"

namespace TrenchBroom {
    namespace Model {
        TEST(BrushGeometryTest, constructWithEmptyFaceList) {
            const FloatType s = 8192.0;
            const Vec3 worldSize2(s, s, s);
            const BBox3 worldBounds(-worldSize2, worldSize2);
            const BrushGeometry geometry(worldBounds);
            
            const BrushVertex::List& vertices = geometry.vertices();
            const BrushEdge::List& edges = geometry.edges();
            const BrushFaceGeometry::List& sides = geometry.sides();
            
            ASSERT_EQ(8u, vertices.size());
            ASSERT_EQ(12u, edges.size());
            ASSERT_EQ(6u, sides.size());
            
            const Vec3 v000(-s, -s, -s);
            const Vec3 v001(-s, -s,  s);
            const Vec3 v010(-s,  s, -s);
            const Vec3 v011(-s,  s,  s);
            const Vec3 v100( s, -s, -s);
            const Vec3 v101( s, -s,  s);
            const Vec3 v110( s,  s, -s);
            const Vec3 v111( s,  s,  s);
            
            Vec3::List topVertices;
            topVertices.push_back(v001);
            topVertices.push_back(v011);
            topVertices.push_back(v111);
            topVertices.push_back(v101);
            
            Vec3::List bottomVertices;
            bottomVertices.push_back(v000);
            bottomVertices.push_back(v100);
            bottomVertices.push_back(v110);
            bottomVertices.push_back(v010);
            
            Vec3::List frontVertices;
            frontVertices.push_back(v000);
            frontVertices.push_back(v001);
            frontVertices.push_back(v101);
            frontVertices.push_back(v100);
            
            Vec3::List backVertices;
            backVertices.push_back(v010);
            backVertices.push_back(v110);
            backVertices.push_back(v111);
            backVertices.push_back(v011);
            
            Vec3::List leftVertices;
            leftVertices.push_back(v000);
            leftVertices.push_back(v010);
            leftVertices.push_back(v011);
            leftVertices.push_back(v001);
            
            Vec3::List rightVertices;
            rightVertices.push_back(v100);
            rightVertices.push_back(v101);
            rightVertices.push_back(v111);
            rightVertices.push_back(v110);
            
            ASSERT_NE(vertices.end(), findBrushVertex(vertices, v000));
            ASSERT_NE(vertices.end(), findBrushVertex(vertices, v001));
            ASSERT_NE(vertices.end(), findBrushVertex(vertices, v010));
            ASSERT_NE(vertices.end(), findBrushVertex(vertices, v011));
            ASSERT_NE(vertices.end(), findBrushVertex(vertices, v100));
            ASSERT_NE(vertices.end(), findBrushVertex(vertices, v101));
            ASSERT_NE(vertices.end(), findBrushVertex(vertices, v110));
            ASSERT_NE(vertices.end(), findBrushVertex(vertices, v111));
            
            ASSERT_NE(edges.end(), findBrushEdge(edges, v000, v001));
            ASSERT_NE(edges.end(), findBrushEdge(edges, v000, v010));
            ASSERT_NE(edges.end(), findBrushEdge(edges, v000, v100));
            ASSERT_NE(edges.end(), findBrushEdge(edges, v001, v011));
            ASSERT_NE(edges.end(), findBrushEdge(edges, v001, v101));
            ASSERT_NE(edges.end(), findBrushEdge(edges, v010, v011));
            ASSERT_NE(edges.end(), findBrushEdge(edges, v010, v110));
            ASSERT_NE(edges.end(), findBrushEdge(edges, v011, v111));
            ASSERT_NE(edges.end(), findBrushEdge(edges, v100, v101));
            ASSERT_NE(edges.end(), findBrushEdge(edges, v100, v110));
            ASSERT_NE(edges.end(), findBrushEdge(edges, v101, v111));
            ASSERT_NE(edges.end(), findBrushEdge(edges, v110, v111));
            
            ASSERT_NE(sides.end(), findBrushFaceGeometry(sides, topVertices));
            ASSERT_NE(sides.end(), findBrushFaceGeometry(sides, bottomVertices));
            ASSERT_NE(sides.end(), findBrushFaceGeometry(sides, frontVertices));
            ASSERT_NE(sides.end(), findBrushFaceGeometry(sides, backVertices));
            ASSERT_NE(sides.end(), findBrushFaceGeometry(sides, leftVertices));
            ASSERT_NE(sides.end(), findBrushFaceGeometry(sides, rightVertices));
        }
        
        TEST(BrushGeometryTest, buildCuboid) {
            const BBox3 cuboid(Vec3(-2.0, -3.0, -3.0), Vec3(6.0, 8.0, 12.0));
            
            BrushFace* top = new QuakeBrushFace(Vec3(0.0, 0.0, cuboid.max.z()),
                                                Vec3(0.0, 1.0, cuboid.max.z()),
                                                Vec3(1.0, 0.0, cuboid.max.z()));
            BrushFace* bottom = new QuakeBrushFace(Vec3(0.0, 0.0, cuboid.min.z()),
                                                   Vec3(1.0, 0.0, cuboid.min.z()),
                                                   Vec3(0.0, 1.0, cuboid.min.z()));
            BrushFace* front = new QuakeBrushFace(Vec3(0.0, cuboid.min.y(),  0.0),
                                                  Vec3(1.0, cuboid.min.y(),  0.0),
                                                  Vec3(0.0, cuboid.min.y(), -1.0));
            BrushFace* back = new QuakeBrushFace(Vec3( 0.0, cuboid.max.y(),  0.0),
                                                 Vec3(-1.0, cuboid.max.y(),  0.0),
                                                 Vec3( 0.0, cuboid.max.y(), -1.0));
            BrushFace* left = new QuakeBrushFace(Vec3(cuboid.min.x(),  0.0,  0.0),
                                                 Vec3(cuboid.min.x(), -1.0,  0.0),
                                                 Vec3(cuboid.min.x(),  0.0, -1.0));
            BrushFace* right = new QuakeBrushFace(Vec3(cuboid.max.x(), 0.0,  0.0),
                                                  Vec3(cuboid.max.x(), 1.0,  0.0),
                                                  Vec3(cuboid.max.x(), 0.0, -1.0));
            
            BrushFaceList faces;
            faces.push_back(top);
            faces.push_back(bottom);
            faces.push_back(front);
            faces.push_back(back);
            faces.push_back(left);
            faces.push_back(right);
            
            const BBox3 worldBounds(-8192.0, 8192.0);
            BrushGeometry geometry(worldBounds);
            const BrushGeometry::AddFaceResult result = geometry.addFaces(faces);
            
            ASSERT_EQ(BrushGeometry::BrushIsSplit, result.resultCode);
            ASSERT_EQ(6u, result.addedFaces.size());
            ASSERT_TRUE(result.droppedFaces.empty());
            
            const BrushVertex::List& vertices = geometry.vertices();
            const BrushEdge::List& edges = geometry.edges();
            const BrushFaceGeometry::List& sides = geometry.sides();
            
            ASSERT_EQ(8u, vertices.size());
            ASSERT_EQ(12u, edges.size());
            ASSERT_EQ(6u, sides.size());
            
            const Vec3 v000(cuboid.min.x(), cuboid.min.y(), cuboid.min.z());
            const Vec3 v001(cuboid.min.x(), cuboid.min.y(), cuboid.max.z());
            const Vec3 v010(cuboid.min.x(), cuboid.max.y(), cuboid.min.z());
            const Vec3 v011(cuboid.min.x(), cuboid.max.y(), cuboid.max.z());
            const Vec3 v100(cuboid.max.x(), cuboid.min.y(), cuboid.min.z());
            const Vec3 v101(cuboid.max.x(), cuboid.min.y(), cuboid.max.z());
            const Vec3 v110(cuboid.max.x(), cuboid.max.y(), cuboid.min.z());
            const Vec3 v111(cuboid.max.x(), cuboid.max.y(), cuboid.max.z());
            
            Vec3::List topVertices;
            topVertices.push_back(v001);
            topVertices.push_back(v011);
            topVertices.push_back(v111);
            topVertices.push_back(v101);
            
            Vec3::List bottomVertices;
            bottomVertices.push_back(v000);
            bottomVertices.push_back(v100);
            bottomVertices.push_back(v110);
            bottomVertices.push_back(v010);
            
            Vec3::List frontVertices;
            frontVertices.push_back(v000);
            frontVertices.push_back(v001);
            frontVertices.push_back(v101);
            frontVertices.push_back(v100);
            
            Vec3::List backVertices;
            backVertices.push_back(v010);
            backVertices.push_back(v110);
            backVertices.push_back(v111);
            backVertices.push_back(v011);
            
            Vec3::List leftVertices;
            leftVertices.push_back(v000);
            leftVertices.push_back(v010);
            leftVertices.push_back(v011);
            leftVertices.push_back(v001);
            
            Vec3::List rightVertices;
            rightVertices.push_back(v100);
            rightVertices.push_back(v101);
            rightVertices.push_back(v111);
            rightVertices.push_back(v110);
            
            ASSERT_NE(vertices.end(), findBrushVertex(vertices, v000));
            ASSERT_NE(vertices.end(), findBrushVertex(vertices, v001));
            ASSERT_NE(vertices.end(), findBrushVertex(vertices, v010));
            ASSERT_NE(vertices.end(), findBrushVertex(vertices, v011));
            ASSERT_NE(vertices.end(), findBrushVertex(vertices, v100));
            ASSERT_NE(vertices.end(), findBrushVertex(vertices, v101));
            ASSERT_NE(vertices.end(), findBrushVertex(vertices, v110));
            ASSERT_NE(vertices.end(), findBrushVertex(vertices, v111));
            
            ASSERT_NE(edges.end(), findBrushEdge(edges, v000, v001));
            ASSERT_NE(edges.end(), findBrushEdge(edges, v000, v010));
            ASSERT_NE(edges.end(), findBrushEdge(edges, v000, v100));
            ASSERT_NE(edges.end(), findBrushEdge(edges, v001, v011));
            ASSERT_NE(edges.end(), findBrushEdge(edges, v001, v101));
            ASSERT_NE(edges.end(), findBrushEdge(edges, v010, v011));
            ASSERT_NE(edges.end(), findBrushEdge(edges, v010, v110));
            ASSERT_NE(edges.end(), findBrushEdge(edges, v011, v111));
            ASSERT_NE(edges.end(), findBrushEdge(edges, v100, v101));
            ASSERT_NE(edges.end(), findBrushEdge(edges, v100, v110));
            ASSERT_NE(edges.end(), findBrushEdge(edges, v101, v111));
            ASSERT_NE(edges.end(), findBrushEdge(edges, v110, v111));
            
            ASSERT_NE(sides.end(), findBrushFaceGeometry(sides, topVertices));
            ASSERT_NE(sides.end(), findBrushFaceGeometry(sides, bottomVertices));
            ASSERT_NE(sides.end(), findBrushFaceGeometry(sides, frontVertices));
            ASSERT_NE(sides.end(), findBrushFaceGeometry(sides, backVertices));
            ASSERT_NE(sides.end(), findBrushFaceGeometry(sides, leftVertices));
            ASSERT_NE(sides.end(), findBrushFaceGeometry(sides, rightVertices));
            
            VectorUtils::clearAndDelete(faces);
        }
    }
}
