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

#include "Exceptions.h"
#include "TrenchBroom.h"
#include "VecMath.h"
#include "CollectionUtils.h"
#include "Model/BrushEdge.h"
#include "Model/BrushFaceGeometry.h"
#include "Model/BrushVertex.h"
#include "TestUtils.h"

#include <algorithm>

namespace TrenchBroom {
    namespace Model {
        inline void updateMarks(const BrushVertex::List& vertices, const BrushEdge::List& edges, const Plane3& plane) {
            BrushVertex::List::const_iterator vIt, vEnd;
            for (vIt = vertices.begin(), vEnd = vertices.end(); vIt != vEnd; ++vIt) {
                BrushVertex& vertex = **vIt;
                vertex.updateMark(plane);
            }
            
            BrushEdge::List::const_iterator eIt, eEnd;
            for (eIt = edges.begin(), eEnd = edges.end(); eIt != eEnd; ++eIt) {
                BrushEdge& edge = **eIt;
                edge.updateMark();
            }
        }
        
        inline BrushVertex::List splitEdges(const BrushEdge::List& edges, const Plane3& plane) {
            BrushVertex::List newVertices;
            BrushEdge::List::const_iterator eIt, eEnd;
            for (eIt = edges.begin(), eEnd = edges.end(); eIt != eEnd; ++eIt) {
                BrushEdge& edge = **eIt;
                if (edge.mark() == BrushEdge::Split)
                    newVertices.push_back(edge.split(plane));
            }
            return newVertices;
        }
        
        TEST(BrushFaceGeometryTest, getMark) {
            BrushVertex* v1 = new BrushVertex(Vec3( 0.0,  0.0, 0.0));
            BrushVertex* v2 = new BrushVertex(Vec3( 0.0, 10.0, 0.0));
            BrushVertex* v3 = new BrushVertex(Vec3(10.0, 10.0, 0.0));
            BrushVertex* v4 = new BrushVertex(Vec3(10.0,  0.0, 0.0));
            
            BrushVertex::List vertices;
            vertices.push_back(v1);
            vertices.push_back(v2);
            vertices.push_back(v3);
            vertices.push_back(v4);
            
            BrushEdge* e1 = new BrushEdge(v1, v2);
            BrushEdge* e2 = new BrushEdge(v2, v3);
            BrushEdge* e3 = new BrushEdge(v3, v4);
            BrushEdge* e4 = new BrushEdge(v4, v1);
            
            BrushEdge::List edges;
            edges.push_back(e1);
            edges.push_back(e2);
            edges.push_back(e3);
            edges.push_back(e4);
            
            BrushFaceGeometry face;
            face.addForwardEdge(e1);
            face.addForwardEdge(e2);
            face.addForwardEdge(e3);
            face.addForwardEdge(e4);

            updateMarks(vertices, edges, Plane3(11.0, Vec3::PosX));
            ASSERT_EQ(BrushFaceGeometry::Keep, face.mark());

            updateMarks(vertices, edges, Plane3(10.0, Vec3::PosX));
            ASSERT_EQ(BrushFaceGeometry::Keep, face.mark());

            updateMarks(vertices, edges, Plane3(9.0, Vec3::PosX));
            ASSERT_EQ(BrushFaceGeometry::Split, face.mark());

            updateMarks(vertices, edges, Plane3(1.0, Vec3::PosX));
            ASSERT_EQ(BrushFaceGeometry::Split, face.mark());

            updateMarks(vertices, edges, Plane3(0.0, Vec3::PosX));
            ASSERT_EQ(BrushFaceGeometry::Drop, face.mark());

            updateMarks(vertices, edges, Plane3(-1.0, Vec3::PosX));
            ASSERT_EQ(BrushFaceGeometry::Drop, face.mark());
            
            VectorUtils::clearAndDelete(edges);
            VectorUtils::clearAndDelete(vertices);
        }
        
        TEST(BrushFaceGeometryTest, splitSquareVerticallyAndSplitTwoEdges) {
            BrushVertex* v1 = new BrushVertex(Vec3( 0.0,  0.0, 0.0));
            BrushVertex* v2 = new BrushVertex(Vec3( 0.0, 10.0, 0.0));
            BrushVertex* v3 = new BrushVertex(Vec3(10.0, 10.0, 0.0));
            BrushVertex* v4 = new BrushVertex(Vec3(10.0,  0.0, 0.0));
            
            BrushVertex::List vertices;
            vertices.push_back(v1);
            vertices.push_back(v2);
            vertices.push_back(v3);
            vertices.push_back(v4);
            
            BrushEdge* e1 = new BrushEdge(v1, v2);
            BrushEdge* e2 = new BrushEdge(v2, v3);
            BrushEdge* e3 = new BrushEdge(v3, v4);
            BrushEdge* e4 = new BrushEdge(v4, v1);
            
            BrushEdge::List edges;
            edges.push_back(e1);
            edges.push_back(e2);
            edges.push_back(e3);
            edges.push_back(e4);
            
            BrushFaceGeometry* face = new BrushFaceGeometry();
            face->addForwardEdge(e1);
            face->addForwardEdge(e2);
            face->addForwardEdge(e3);
            face->addForwardEdge(e4);

            const Plane3 plane(5.0, Vec3::PosX);
            updateMarks(vertices, edges, plane);
            BrushVertex::List newVertices = splitEdges(edges, plane);
            BrushEdge* newEdge = face->splitUsingEdgeMarks();
            
            ASSERT_TRUE(newEdge != NULL);
            ASSERT_VEC_EQ(Vec3(5.0,  0.0, 0.0), newEdge->start()->position());
            ASSERT_VEC_EQ(Vec3(5.0, 10.0, 0.0), newEdge->end()->position());
            ASSERT_EQ(4u, face->vertices().size());
            ASSERT_EQ(4u, face->edges().size());
            
            BrushFaceGeometry::List faces;
            faces.push_back(face);
            
            Vec3::List newPositions;
            newPositions.push_back(Vec3(0.0,  0.0, 0.0));
            newPositions.push_back(Vec3(0.0, 10.0, 0.0));
            newPositions.push_back(Vec3(5.0, 10.0, 0.0));
            newPositions.push_back(Vec3(5.0,  0.0, 0.0));
            
            ASSERT_EQ(faces.begin(), findBrushFaceGeometry(faces, newPositions));
            
            delete face;
            delete newEdge;
            VectorUtils::clearAndDelete(edges);
            VectorUtils::clearAndDelete(vertices);
            VectorUtils::clearAndDelete(newVertices);
        }
        
        TEST(BrushFaceGeometryTest, splitSquareAndSplitOneEdge) {
            BrushVertex* v1 = new BrushVertex(Vec3( 0.0,  0.0, 0.0));
            BrushVertex* v2 = new BrushVertex(Vec3( 0.0, 10.0, 0.0));
            BrushVertex* v3 = new BrushVertex(Vec3(10.0, 10.0, 0.0));
            BrushVertex* v4 = new BrushVertex(Vec3(10.0,  0.0, 0.0));
            
            BrushVertex::List vertices;
            vertices.push_back(v1);
            vertices.push_back(v2);
            vertices.push_back(v3);
            vertices.push_back(v4);
            
            BrushEdge* e1 = new BrushEdge(v1, v2);
            BrushEdge* e2 = new BrushEdge(v2, v3);
            BrushEdge* e3 = new BrushEdge(v3, v4);
            BrushEdge* e4 = new BrushEdge(v4, v1);
            
            BrushEdge::List edges;
            edges.push_back(e1);
            edges.push_back(e2);
            edges.push_back(e3);
            edges.push_back(e4);
            
            BrushFaceGeometry* face = new BrushFaceGeometry();
            face->addForwardEdge(e1);
            face->addForwardEdge(e2);
            face->addForwardEdge(e3);
            face->addForwardEdge(e4);
            
            const Plane3 plane(Vec3(10.0, 10.0, 0.0), Vec3(2.0, -1.0, 0.0).normalized());
            updateMarks(vertices, edges, plane);
            BrushVertex::List newVertices = splitEdges(edges, plane);
            BrushEdge* newEdge = face->splitUsingEdgeMarks();
            
            ASSERT_TRUE(newEdge != NULL);
            ASSERT_VEC_EQ(Vec3( 5.0,  0.0, 0.0), newEdge->start()->position());
            ASSERT_VEC_EQ(Vec3(10.0, 10.0, 0.0), newEdge->end()->position());
            ASSERT_EQ(4u, face->vertices().size());
            ASSERT_EQ(4u, face->edges().size());
            
            BrushFaceGeometry::List faces;
            faces.push_back(face);
            
            Vec3::List newPositions;
            newPositions.push_back(Vec3( 0.0,  0.0, 0.0));
            newPositions.push_back(Vec3( 0.0, 10.0, 0.0));
            newPositions.push_back(Vec3(10.0, 10.0, 0.0));
            newPositions.push_back(Vec3( 5.0,  0.0, 0.0));
            
            ASSERT_EQ(faces.begin(), findBrushFaceGeometry(faces, newPositions));
            
            delete face;
            delete newEdge;
            VectorUtils::clearAndDelete(edges);
            VectorUtils::clearAndDelete(vertices);
            VectorUtils::clearAndDelete(newVertices);
        }

        TEST(BrushFaceGeometryTest, splitSquareAndSplitNoEdge) {
            BrushVertex* v1 = new BrushVertex(Vec3( 0.0,  0.0, 0.0));
            BrushVertex* v2 = new BrushVertex(Vec3( 0.0, 10.0, 0.0));
            BrushVertex* v3 = new BrushVertex(Vec3(10.0, 10.0, 0.0));
            BrushVertex* v4 = new BrushVertex(Vec3(10.0,  0.0, 0.0));
            
            BrushVertex::List vertices;
            vertices.push_back(v1);
            vertices.push_back(v2);
            vertices.push_back(v3);
            vertices.push_back(v4);
            
            BrushEdge* e1 = new BrushEdge(v1, v2);
            BrushEdge* e2 = new BrushEdge(v2, v3);
            BrushEdge* e3 = new BrushEdge(v3, v4);
            BrushEdge* e4 = new BrushEdge(v4, v1);
            
            BrushEdge::List edges;
            edges.push_back(e1);
            edges.push_back(e2);
            edges.push_back(e3);
            edges.push_back(e4);
            
            BrushFaceGeometry* face = new BrushFaceGeometry();
            face->addForwardEdge(e1);
            face->addForwardEdge(e2);
            face->addForwardEdge(e3);
            face->addForwardEdge(e4);
            
            const Plane3 plane(Vec3(10.0, 10.0, 0.0), Vec3(1.0, -1.0, 0.0).normalized());
            updateMarks(vertices, edges, plane);
            BrushVertex::List newVertices = splitEdges(edges, plane);
            BrushEdge* newEdge = face->splitUsingEdgeMarks();
            
            ASSERT_TRUE(newEdge != NULL);
            ASSERT_VEC_EQ(Vec3( 0.0,  0.0, 0.0), newEdge->start()->position());
            ASSERT_VEC_EQ(Vec3(10.0, 10.0, 0.0), newEdge->end()->position());
            ASSERT_EQ(3u, face->vertices().size());
            ASSERT_EQ(3u, face->edges().size());
            
            BrushFaceGeometry::List faces;
            faces.push_back(face);
            
            Vec3::List newPositions;
            newPositions.push_back(Vec3( 0.0,  0.0, 0.0));
            newPositions.push_back(Vec3( 0.0, 10.0, 0.0));
            newPositions.push_back(Vec3(10.0, 10.0, 0.0));
            
            ASSERT_EQ(faces.begin(), findBrushFaceGeometry(faces, newPositions));
            
            delete face;
            delete newEdge;
            VectorUtils::clearAndDelete(edges);
            VectorUtils::clearAndDelete(vertices);
            VectorUtils::clearAndDelete(newVertices);
        }
        
        TEST(BrushFaceGeometryTest, findUndecidedEdge) {
            BrushVertex* v1 = new BrushVertex(Vec3( 0.0,  0.0, 0.0));
            BrushVertex* v2 = new BrushVertex(Vec3( 0.0, 10.0, 0.0));
            BrushVertex* v3 = new BrushVertex(Vec3(10.0, 10.0, 0.0));
            BrushVertex* v4 = new BrushVertex(Vec3(10.0,  0.0, 0.0));
            
            BrushVertex::List vertices;
            vertices.push_back(v1);
            vertices.push_back(v2);
            vertices.push_back(v3);
            vertices.push_back(v4);
            
            BrushEdge* e1 = new BrushEdge(v1, v2);
            BrushEdge* e2 = new BrushEdge(v2, v3);
            BrushEdge* e3 = new BrushEdge(v3, v4);
            BrushEdge* e4 = new BrushEdge(v4, v1);
            
            BrushEdge::List edges;
            edges.push_back(e1);
            edges.push_back(e2);
            edges.push_back(e3);
            edges.push_back(e4);
            
            BrushFaceGeometry* face = new BrushFaceGeometry();
            face->addForwardEdge(e1);
            face->addForwardEdge(e2);
            face->addForwardEdge(e3);
            face->addForwardEdge(e4);
            
            const Plane3 plane1(10.0, Vec3::PosX);
            updateMarks(vertices, edges, plane1);
            ASSERT_EQ(BrushFaceGeometry::Keep, face->mark());
            ASSERT_EQ(e3, face->findUndecidedEdge());
            
            const Plane3 plane2(0.0, Vec3::PosX);
            updateMarks(vertices, edges, plane2);
            ASSERT_EQ(BrushFaceGeometry::Drop, face->mark());
            ASSERT_EQ(e1, face->findUndecidedEdge());
            
            delete face;
            VectorUtils::clearAndDelete(edges);
            VectorUtils::clearAndDelete(vertices);
        }

        TEST(BrushFaceGeometryTest, addForwardEdge) {
            BrushVertex* v1 = new BrushVertex(Vec3(1.0, 2.0, 3.0));
            BrushVertex* v2 = new BrushVertex(Vec3(2.0, 3.0, 4.0));
            BrushVertex* v3 = new BrushVertex(Vec3(3.0, 4.0, 5.0));
            
            BrushEdge* e1 = new BrushEdge(v1, v2);
            BrushEdge* e2 = new BrushEdge(v2, v3);
            BrushEdge* e3 = new BrushEdge(v3, v1);

            BrushFaceGeometry face;
            face.addForwardEdge(e1);
            
            ASSERT_THROW(face.addForwardEdge(e1), GeometryException);
            ASSERT_THROW(face.addForwardEdge(e3), GeometryException);
            
            face.addForwardEdge(e2);
            face.addForwardEdge(e3);
            
            const BrushEdge::List& edges = face.edges();
            ASSERT_EQ(e1, edges[0]);
            ASSERT_EQ(e2, edges[1]);
            ASSERT_EQ(e3, edges[2]);
            
            const BrushVertex::List& vertices = face.vertices();
            ASSERT_EQ(v1, vertices[0]);
            ASSERT_EQ(v2, vertices[1]);
            ASSERT_EQ(v3, vertices[2]);
            
            ASSERT_EQ(NULL, e1->left());
            ASSERT_EQ(NULL, e2->left());
            ASSERT_EQ(NULL, e3->left());
            ASSERT_EQ(&face, e1->right());
            ASSERT_EQ(&face, e2->right());
            ASSERT_EQ(&face, e3->right());
            
            delete e1;
            delete e2;
            delete e3;
            delete v1;
            delete v2;
            delete v3;
        }
        
        TEST(BrushFaceGeometryTest, addBackwardEdge) {
            BrushVertex* v1 = new BrushVertex(Vec3(1.0, 2.0, 3.0));
            BrushVertex* v2 = new BrushVertex(Vec3(2.0, 3.0, 4.0));
            BrushVertex* v3 = new BrushVertex(Vec3(3.0, 4.0, 5.0));
            
            BrushEdge* e1 = new BrushEdge(v2, v1);
            BrushEdge* e2 = new BrushEdge(v3, v2);
            BrushEdge* e3 = new BrushEdge(v1, v3);
            
            BrushFaceGeometry face;
            face.addBackwardEdge(e1);
            
            ASSERT_THROW(face.addBackwardEdge(e1), GeometryException);
            ASSERT_THROW(face.addBackwardEdge(e3), GeometryException);
            
            face.addBackwardEdge(e2);
            face.addBackwardEdge(e3);
            
            const BrushEdge::List& edges = face.edges();
            ASSERT_EQ(e1, edges[0]);
            ASSERT_EQ(e2, edges[1]);
            ASSERT_EQ(e3, edges[2]);
            
            const BrushVertex::List& vertices = face.vertices();
            ASSERT_EQ(v1, vertices[0]);
            ASSERT_EQ(v2, vertices[1]);
            ASSERT_EQ(v3, vertices[2]);
            
            ASSERT_EQ(NULL, e1->right());
            ASSERT_EQ(NULL, e2->right());
            ASSERT_EQ(NULL, e3->right());
            ASSERT_EQ(&face, e1->left());
            ASSERT_EQ(&face, e2->left());
            ASSERT_EQ(&face, e3->left());
            
            delete e1;
            delete e2;
            delete e3;
            delete v1;
            delete v2;
            delete v3;
        }
        
        TEST(BrushFaceGeometryTest, isClosed) {
            BrushVertex* v1 = new BrushVertex(Vec3(1.0, 2.0, 3.0));
            BrushVertex* v2 = new BrushVertex(Vec3(2.0, 3.0, 4.0));
            BrushVertex* v3 = new BrushVertex(Vec3(3.0, 4.0, 5.0));
            
            BrushEdge* e1 = new BrushEdge(v1, v2);
            BrushEdge* e2 = new BrushEdge(v2, v3);
            BrushEdge* e3 = new BrushEdge(v3, v1);
            
            BrushFaceGeometry face;
            ASSERT_FALSE(face.isClosed());
            face.addForwardEdge(e1);
            ASSERT_FALSE(face.isClosed());
            face.addForwardEdge(e2);
            ASSERT_FALSE(face.isClosed());
            face.addForwardEdge(e3);
            ASSERT_TRUE(face.isClosed());
            
            delete e1;
            delete e2;
            delete e3;
            delete v1;
            delete v2;
            delete v3;
        }
        
        TEST(BrushFaceGeometryTest, hasVertexPositions) {
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
            
            BrushVertex::List vertices;
            vertices.push_back(v1);
            vertices.push_back(v2);
            vertices.push_back(v3);
            vertices.push_back(v4);
            vertices.push_back(v5);
            
            BrushEdge::List edges;
            edges.push_back(new BrushEdge(v1, v2));
            edges.push_back(new BrushEdge(v2, v3));
            edges.push_back(new BrushEdge(v3, v4));
            edges.push_back(new BrushEdge(v4, v5));
            edges.push_back(new BrushEdge(v5, v1));
            
            BrushFaceGeometry face;
            face.addForwardEdges(edges);
            
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
        
        TEST(BrushFaceGeometryTest, findBrushFaceGeometry) {
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
            
            BrushVertex::List vertices;
            vertices.push_back(v1);
            vertices.push_back(v2);
            vertices.push_back(v3);
            vertices.push_back(v4);
            vertices.push_back(v5);
            
            BrushEdge::List edges1;
            edges1.push_back(new BrushEdge(v1, v2));
            edges1.push_back(new BrushEdge(v2, v3));
            edges1.push_back(new BrushEdge(v3, v4));
            edges1.push_back(new BrushEdge(v4, v5));
            edges1.push_back(new BrushEdge(v5, v1));

            BrushEdge::List edges2;
            edges2.push_back(new BrushEdge(v1, v2));
            edges2.push_back(new BrushEdge(v2, v3));
            edges2.push_back(new BrushEdge(v3, v1));

            BrushEdge::List edges3;
            edges3.push_back(new BrushEdge(v3, v4));
            edges3.push_back(new BrushEdge(v4, v5));
            edges3.push_back(new BrushEdge(v5, v3));
            
            BrushFaceGeometry* faceGeometry1 = new BrushFaceGeometry();
            faceGeometry1->addForwardEdges(edges1);
            BrushFaceGeometry* faceGeometry2 = new BrushFaceGeometry();
            faceGeometry2->addForwardEdges(edges2);
            BrushFaceGeometry* faceGeometry3 = new BrushFaceGeometry();
            faceGeometry3->addForwardEdges(edges3);
            
            BrushFaceGeometry::List faceGeometries;
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
                VectorUtils::shiftLeft(positions1, i);
                VectorUtils::shiftLeft(positions2, i);
                VectorUtils::shiftLeft(positions3, i);
                VectorUtils::shiftLeft(positions4, i);

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
