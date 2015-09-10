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

#include "TestUtils.h"

#include "Model/Brush.h"
#include "Model/BrushBuilder.h"
#include "Model/BrushContentTypeBuilder.h"
#include "Model/BrushFace.h"
#include "Model/Hit.h"
#include "Model/MapFormat.h"
#include "Model/ModelFactoryImpl.h"
#include "Model/PickResult.h"
#include "Model/World.h"

#include <algorithm>

namespace TrenchBroom {
    namespace Model {
        TEST(BrushTest, constructBrushWithRedundantFaces) {
            const BBox3 worldBounds(4096.0);
            
            BrushFaceList faces;
            faces.push_back(BrushFace::createParaxial(Vec3(0.0, 0.0, 0.0),
                                                      Vec3(1.0, 0.0, 0.0),
                                                      Vec3(0.0, 1.0, 0.0)));
            faces.push_back(BrushFace::createParaxial(Vec3(0.0, 0.0, 0.0),
                                                      Vec3(1.0, 0.0, 0.0),
                                                      Vec3(0.0, 1.0, 0.0)));
            faces.push_back(BrushFace::createParaxial(Vec3(0.0, 0.0, 0.0),
                                                      Vec3(1.0, 0.0, 0.0),
                                                      Vec3(0.0, 1.0, 0.0)));
            
            Brush brush(worldBounds, faces);
            const BrushFaceList& brushFaces = brush.faces();
            ASSERT_EQ(1u, brushFaces.size());
            ASSERT_EQ(faces[0], brushFaces[0]);
        }
        
        TEST(BrushTest, constructBrushWithFaces) {
            const BBox3 worldBounds(4096.0);
            
            // build a cube with length 16 at the origin
            BrushFace* left = BrushFace::createParaxial(Vec3(0.0, 0.0, 0.0),
                                                        Vec3(0.0, 1.0, 0.0),
                                                        Vec3(0.0, 0.0, 1.0));
            BrushFace* right = BrushFace::createParaxial(Vec3(16.0, 0.0, 0.0),
                                                         Vec3(16.0, 0.0, 1.0),
                                                         Vec3(16.0, 1.0, 0.0));
            BrushFace* front = BrushFace::createParaxial(Vec3(0.0, 0.0, 0.0),
                                                         Vec3(0.0, 0.0, 1.0),
                                                         Vec3(1.0, 0.0, 0.0));
            BrushFace* back = BrushFace::createParaxial(Vec3(0.0, 16.0, 0.0),
                                                        Vec3(1.0, 16.0, 0.0),
                                                        Vec3(0.0, 16.0, 1.0));
            BrushFace* top = BrushFace::createParaxial(Vec3(0.0, 0.0, 16.0),
                                                       Vec3(0.0, 1.0, 16.0),
                                                       Vec3(1.0, 0.0, 16.0));
            BrushFace* bottom = BrushFace::createParaxial(Vec3(0.0, 0.0, 0.0),
                                                          Vec3(1.0, 0.0, 0.0),
                                                          Vec3(0.0, 1.0, 0.0));
            
            BrushFaceList faces;
            faces.push_back(left);
            faces.push_back(right);
            faces.push_back(front);
            faces.push_back(back);
            faces.push_back(top);
            faces.push_back(bottom);
            
            Brush brush(worldBounds, faces);
            const BrushFaceList& brushFaces = brush.faces();
            ASSERT_EQ(6u, brushFaces.size());
            for (size_t i = 0; i < faces.size(); i++)
                ASSERT_EQ(faces[i], brushFaces[i]);
        }
        
        TEST(BrushTest, pick) {
            const BBox3 worldBounds(4096.0);
            
            // build a cube with length 16 at the origin
            BrushFace* left = BrushFace::createParaxial(Vec3(0.0, 0.0, 0.0),
                                                        Vec3(0.0, 1.0, 0.0),
                                                        Vec3(0.0, 0.0, 1.0));
            BrushFace* right = BrushFace::createParaxial(Vec3(16.0, 0.0, 0.0),
                                                         Vec3(16.0, 0.0, 1.0),
                                                         Vec3(16.0, 1.0, 0.0));
            BrushFace* front = BrushFace::createParaxial(Vec3(0.0, 0.0, 0.0),
                                                         Vec3(0.0, 0.0, 1.0),
                                                         Vec3(1.0, 0.0, 0.0));
            BrushFace* back = BrushFace::createParaxial(Vec3(0.0, 16.0, 0.0),
                                                        Vec3(1.0, 16.0, 0.0),
                                                        Vec3(0.0, 16.0, 1.0));
            BrushFace* top = BrushFace::createParaxial(Vec3(0.0, 0.0, 16.0),
                                                       Vec3(0.0, 1.0, 16.0),
                                                       Vec3(1.0, 0.0, 16.0));
            BrushFace* bottom = BrushFace::createParaxial(Vec3(0.0, 0.0, 0.0),
                                                          Vec3(1.0, 0.0, 0.0),
                                                          Vec3(0.0, 1.0, 0.0));
            
            BrushFaceList faces;
            faces.push_back(left);
            faces.push_back(right);
            faces.push_back(front);
            faces.push_back(back);
            faces.push_back(top);
            faces.push_back(bottom);
            
            Brush brush(worldBounds, faces);
            
            PickResult hits1;
            brush.pick(Ray3(Vec3(8.0, -8.0, 8.0), Vec3::PosY), hits1);
            ASSERT_EQ(1u, hits1.size());
            
            Hit hit1 = hits1.all().front();
            ASSERT_DOUBLE_EQ(8.0, hit1.distance());
            BrushFace* face1 = hit1.target<BrushFace*>();
            ASSERT_EQ(front, face1);
            
            PickResult hits2;
            brush.pick(Ray3(Vec3(8.0, -8.0, 8.0), Vec3::NegY), hits2);
            ASSERT_TRUE(hits2.empty());
        }
        
        TEST(BrushTest, partialSelectionAfterAdd) {
            const BBox3 worldBounds(4096.0);
            
            // build a cube with length 16 at the origin
            BrushFace* left = BrushFace::createParaxial(Vec3(0.0, 0.0, 0.0),
                                                        Vec3(0.0, 1.0, 0.0),
                                                        Vec3(0.0, 0.0, 1.0));
            BrushFace* right = BrushFace::createParaxial(Vec3(16.0, 0.0, 0.0),
                                                         Vec3(16.0, 0.0, 1.0),
                                                         Vec3(16.0, 1.0, 0.0));
            BrushFace* front = BrushFace::createParaxial(Vec3(0.0, 0.0, 0.0),
                                                         Vec3(0.0, 0.0, 1.0),
                                                         Vec3(1.0, 0.0, 0.0));
            BrushFace* back = BrushFace::createParaxial(Vec3(0.0, 16.0, 0.0),
                                                        Vec3(1.0, 16.0, 0.0),
                                                        Vec3(0.0, 16.0, 1.0));
            BrushFace* top = BrushFace::createParaxial(Vec3(0.0, 0.0, 16.0),
                                                       Vec3(0.0, 1.0, 16.0),
                                                       Vec3(1.0, 0.0, 16.0));
            BrushFace* bottom = BrushFace::createParaxial(Vec3(0.0, 0.0, 0.0),
                                                          Vec3(1.0, 0.0, 0.0),
                                                          Vec3(0.0, 1.0, 0.0));
            
            BrushFaceList faces;
            faces.push_back(left);
            faces.push_back(right);
            faces.push_back(front);
            faces.push_back(back);
            faces.push_back(top);
            faces.push_back(bottom);
            
            Brush brush(worldBounds, faces);
            ASSERT_FALSE(brush.descendantSelected());
            left->select();
            ASSERT_TRUE(brush.descendantSelected());
            right->select();
            left->deselect();
            ASSERT_TRUE(brush.descendantSelected());
            right->deselect();
            ASSERT_FALSE(brush.descendantSelected());
        }
        
        TEST(BrushTest, partialSelectionBeforeAdd) {
            const BBox3 worldBounds(4096.0);
            
            // build a cube with length 16 at the origin
            BrushFace* left = BrushFace::createParaxial(Vec3(0.0, 0.0, 0.0),
                                                        Vec3(0.0, 1.0, 0.0),
                                                        Vec3(0.0, 0.0, 1.0));
            BrushFace* right = BrushFace::createParaxial(Vec3(16.0, 0.0, 0.0),
                                                         Vec3(16.0, 0.0, 1.0),
                                                         Vec3(16.0, 1.0, 0.0));
            BrushFace* front = BrushFace::createParaxial(Vec3(0.0, 0.0, 0.0),
                                                         Vec3(0.0, 0.0, 1.0),
                                                         Vec3(1.0, 0.0, 0.0));
            BrushFace* back = BrushFace::createParaxial(Vec3(0.0, 16.0, 0.0),
                                                        Vec3(1.0, 16.0, 0.0),
                                                        Vec3(0.0, 16.0, 1.0));
            BrushFace* top = BrushFace::createParaxial(Vec3(0.0, 0.0, 16.0),
                                                       Vec3(0.0, 1.0, 16.0),
                                                       Vec3(1.0, 0.0, 16.0));
            BrushFace* bottom = BrushFace::createParaxial(Vec3(0.0, 0.0, 0.0),
                                                          Vec3(1.0, 0.0, 0.0),
                                                          Vec3(0.0, 1.0, 0.0));
            
            BrushFaceList faces;
            faces.push_back(left);
            faces.push_back(right);
            faces.push_back(front);
            faces.push_back(back);
            faces.push_back(top);
            faces.push_back(bottom);
            
            left->select();
            right->select();
            
            Brush brush(worldBounds, faces);
            ASSERT_TRUE(brush.descendantSelected());
            left->deselect();
            ASSERT_TRUE(brush.descendantSelected());
            right->deselect();
            ASSERT_FALSE(brush.descendantSelected());
        }
        
        struct MatchFace {
        private:
            const BrushFace& m_face;
        public:
            MatchFace(const BrushFace& face) :
            m_face(face) {}
            
            bool operator()(const BrushFace* candidate) const {
                for (size_t i = 0; i < 3; ++i)
                    if (candidate->points()[i] != m_face.points()[i])
                        return false;
                if (candidate->selected() != m_face.selected())
                    return false;
                if (candidate->textureName() != m_face.textureName())
                    return false;
                if (candidate->texture() != m_face.texture())
                    return false;
                if (candidate->xOffset() != m_face.xOffset())
                    return false;
                if (candidate->yOffset() != m_face.yOffset())
                    return false;
                if (candidate->rotation() != m_face.rotation())
                    return false;
                if (candidate->xScale() != m_face.xScale())
                    return false;
                if (candidate->yScale() != m_face.yScale())
                    return false;
                if (candidate->surfaceContents() != m_face.surfaceContents())
                    return false;
                if (candidate->surfaceFlags() != m_face.surfaceFlags())
                    return false;
                if (candidate->surfaceValue() != m_face.surfaceValue())
                    return false;
                return true;
            }
        };
        
        static void assertHasFace(const Brush& brush, const BrushFace& face) {
            const BrushFaceList& faces = brush.faces();
            const BrushFaceList::const_iterator it = std::find_if(faces.begin(), faces.end(), MatchFace(face));
            ASSERT_TRUE(it != faces.end());
        }
        
        TEST(BrushTest, clone) {
            const BBox3 worldBounds(4096.0);
            
            // build a cube with length 16 at the origin
            BrushFace* left = BrushFace::createParaxial(Vec3(0.0, 0.0, 0.0),
                                                        Vec3(0.0, 1.0, 0.0),
                                                        Vec3(0.0, 0.0, 1.0));
            BrushFace* right = BrushFace::createParaxial(Vec3(16.0, 0.0, 0.0),
                                                         Vec3(16.0, 0.0, 1.0),
                                                         Vec3(16.0, 1.0, 0.0));
            BrushFace* front = BrushFace::createParaxial(Vec3(0.0, 0.0, 0.0),
                                                         Vec3(0.0, 0.0, 1.0),
                                                         Vec3(1.0, 0.0, 0.0));
            BrushFace* back = BrushFace::createParaxial(Vec3(0.0, 16.0, 0.0),
                                                        Vec3(1.0, 16.0, 0.0),
                                                        Vec3(0.0, 16.0, 1.0));
            BrushFace* top = BrushFace::createParaxial(Vec3(0.0, 0.0, 16.0),
                                                       Vec3(0.0, 1.0, 16.0),
                                                       Vec3(1.0, 0.0, 16.0));
            BrushFace* bottom = BrushFace::createParaxial(Vec3(0.0, 0.0, 0.0),
                                                          Vec3(1.0, 0.0, 0.0),
                                                          Vec3(0.0, 1.0, 0.0));
            
            BrushFaceList faces;
            faces.push_back(left);
            faces.push_back(right);
            faces.push_back(front);
            faces.push_back(back);
            faces.push_back(top);
            faces.push_back(bottom);
            
            Brush original(worldBounds, faces);
            Brush* clone = original.clone(worldBounds);
            
            assertHasFace(*clone, *left);
            assertHasFace(*clone, *right);
            assertHasFace(*clone, *front);
            assertHasFace(*clone, *back);
            assertHasFace(*clone, *top);
            assertHasFace(*clone, *bottom);
            
            delete clone;
        }
        
        TEST(BrushTest, clip) {
            const BBox3 worldBounds(4096.0);
            
            // build a cube with length 16 at the origin
            BrushFace* left = BrushFace::createParaxial(Vec3(0.0, 0.0, 0.0),
                                                        Vec3(0.0, 1.0, 0.0),
                                                        Vec3(0.0, 0.0, 1.0));
            BrushFace* right = BrushFace::createParaxial(Vec3(16.0, 0.0, 0.0),
                                                         Vec3(16.0, 0.0, 1.0),
                                                         Vec3(16.0, 1.0, 0.0));
            BrushFace* front = BrushFace::createParaxial(Vec3(0.0, 0.0, 0.0),
                                                         Vec3(0.0, 0.0, 1.0),
                                                         Vec3(1.0, 0.0, 0.0));
            BrushFace* back = BrushFace::createParaxial(Vec3(0.0, 16.0, 0.0),
                                                        Vec3(1.0, 16.0, 0.0),
                                                        Vec3(0.0, 16.0, 1.0));
            BrushFace* top = BrushFace::createParaxial(Vec3(0.0, 0.0, 16.0),
                                                       Vec3(0.0, 1.0, 16.0),
                                                       Vec3(1.0, 0.0, 16.0));
            BrushFace* bottom = BrushFace::createParaxial(Vec3(0.0, 0.0, 0.0),
                                                          Vec3(1.0, 0.0, 0.0),
                                                          Vec3(0.0, 1.0, 0.0));
            BrushFace* clip = BrushFace::createParaxial(Vec3(8.0, 0.0, 0.0),
                                                        Vec3(8.0, 0.0, 1.0),
                                                        Vec3(8.0, 1.0, 0.0));
            
            BrushFaceList faces;
            faces.push_back(left);
            faces.push_back(right);
            faces.push_back(front);
            faces.push_back(back);
            faces.push_back(top);
            faces.push_back(bottom);
            
            Brush brush(worldBounds, faces);
            ASSERT_TRUE(brush.clip(worldBounds, clip));
            
            ASSERT_EQ(6u, brush.faces().size());
            assertHasFace(brush, *left);
            assertHasFace(brush, *clip);
            assertHasFace(brush, *front);
            assertHasFace(brush, *back);
            assertHasFace(brush, *top);
            assertHasFace(brush, *bottom);
        }
        
        TEST(BrushTest, moveBoundary) {
            const BBox3 worldBounds(4096.0);
            
            // left and right a are slanted!
            BrushFace* left   = BrushFace::createParaxial(Vec3( 0.0,  0.0, 0.0),
                                                          Vec3( 0.0,  1.0, 0.0),
                                                          Vec3( 1.0,  0.0, 1.0));
            BrushFace* right  = BrushFace::createParaxial(Vec3(16.0,  0.0, 0.0),
                                                          Vec3(15.0,  0.0, 1.0),
                                                          Vec3(16.0,  1.0, 0.0));
            BrushFace* front  = BrushFace::createParaxial(Vec3( 0.0,  0.0, 0.0),
                                                          Vec3( 0.0,  0.0, 1.0),
                                                          Vec3( 1.0,  0.0, 0.0));
            BrushFace* back   = BrushFace::createParaxial(Vec3( 0.0, 16.0, 0.0),
                                                          Vec3( 1.0, 16.0, 0.0),
                                                          Vec3( 0.0, 16.0, 1.0));
            BrushFace* top    = BrushFace::createParaxial(Vec3( 0.0,  0.0, 6.0),
                                                          Vec3( 0.0,  1.0, 6.0),
                                                          Vec3( 1.0,  0.0, 6.0));
            BrushFace* bottom = BrushFace::createParaxial(Vec3( 0.0,  0.0, 0.0),
                                                          Vec3( 1.0,  0.0, 0.0),
                                                          Vec3( 0.0,  1.0, 0.0));
            BrushFaceList faces;
            faces.push_back(left);
            faces.push_back(right);
            faces.push_back(front);
            faces.push_back(back);
            faces.push_back(top);
            faces.push_back(bottom);
            
            Brush brush(worldBounds, faces);
            ASSERT_EQ(6u, brush.faces().size());
            
            ASSERT_FALSE(brush.canMoveBoundary(worldBounds, top, Vec3(0.0, 0.0, +16.0)));
            ASSERT_FALSE(brush.canMoveBoundary(worldBounds, top, Vec3(0.0, 0.0, -16.0)));
            ASSERT_FALSE(brush.canMoveBoundary(worldBounds, top, Vec3(0.0, 0.0, +2.0)));
            ASSERT_FALSE(brush.canMoveBoundary(worldBounds, top, Vec3(0.0, 0.0, -6.0)));
            ASSERT_TRUE(brush.canMoveBoundary(worldBounds, top, Vec3(0.0, 0.0, +1.0)));
            ASSERT_TRUE(brush.canMoveBoundary(worldBounds, top, Vec3(0.0, 0.0, -5.0)));
            
            brush.moveBoundary(worldBounds, top, Vec3(0.0, 0.0, 1.0), false);
            ASSERT_EQ(6u, brush.faces().size());
            ASSERT_DOUBLE_EQ(7.0, brush.bounds().size().z());
        }
        
        TEST(BrushTest, moveVertex) {
            const BBox3 worldBounds(4096.0);
            World world(MapFormat::Standard, NULL, worldBounds);
            
            BrushBuilder builder(&world, worldBounds);
            Brush* brush = builder.createCube(64.0, "asdf");

            const Vec3 vertex(32.0, 32.0, 32.0);
            Vec3::List newVertexPositions = brush->moveVertices(worldBounds, Vec3::List(1, vertex), Vec3(-16.0, -16.0, 0.0));
            ASSERT_EQ(1u, newVertexPositions.size());
            ASSERT_VEC_EQ(Vec3(16.0, 16.0, 32.0), newVertexPositions[0]);
            
            newVertexPositions = brush->moveVertices(worldBounds, newVertexPositions, Vec3(16.0, 16.0, 0.0));
            ASSERT_EQ(1u, newVertexPositions.size());
            ASSERT_VEC_EQ(vertex, newVertexPositions[0]);

            delete brush;
        }
        
        TEST(BrushTest, moveEdge) {
            const BBox3 worldBounds(4096.0);
            World world(MapFormat::Standard, NULL, worldBounds);

            BrushBuilder builder(&world, worldBounds);
            Brush* brush = builder.createCube(64.0, "asdf");
            
            const Edge3 edge(Vec3(-32.0, -32.0, -32.0), Vec3(32.0, -32.0, -32.0));
            Edge3::List newEdgePositions = brush->moveEdges(worldBounds, Edge3::List(1, edge), Vec3(-16.0, -16.0, 0.0));
            ASSERT_EQ(1u, newEdgePositions.size());
            ASSERT_EQ(Edge3(Vec3(-48.0, -48.0, -32.0), Vec3(16.0, -48.0, -32.0)), newEdgePositions[0]);
            
            newEdgePositions = brush->moveEdges(worldBounds, newEdgePositions, Vec3(16.0, 16.0, 0.0));
            ASSERT_EQ(1u, newEdgePositions.size());
            ASSERT_EQ(edge, newEdgePositions[0]);
            
            delete brush;
        }
        
        TEST(BrushTest, splitEdge) {
            const BBox3 worldBounds(4096.0);
            World world(MapFormat::Standard, NULL, worldBounds);

            BrushBuilder builder(&world, worldBounds);
            Brush* brush = builder.createCube(64.0, "asdf");

            const Edge3 edge(Vec3(-32.0, -32.0, -32.0), Vec3(32.0, -32.0, -32.0));
            const Vec3 newVertexPosition = brush->splitEdge(worldBounds, edge, Vec3(-16.0, -16.0, 0.0));
            
            ASSERT_VEC_EQ(Vec3(-16.0, -48.0, -32.0), newVertexPosition);
            ASSERT_EQ(9u, brush->vertexCount());
            ASSERT_EQ(15u, brush->edgeCount());
            
            delete brush;
        }
        
        TEST(BrushTest, moveFace) {
            const BBox3 worldBounds(4096.0);
            World world(MapFormat::Standard, NULL, worldBounds);

            BrushBuilder builder(&world, worldBounds);
            Brush* brush = builder.createCube(64.0, "asdf");
            
            Vec3::List vertexPositions(4);
            vertexPositions[0] = Vec3(-32.0, -32.0, +32.0);
            vertexPositions[1] = Vec3(+32.0, -32.0, +32.0);
            vertexPositions[2] = Vec3(+32.0, +32.0, +32.0);
            vertexPositions[3] = Vec3(-32.0, +32.0, +32.0);
            
            const Polygon3 face(vertexPositions);
            
            Polygon3::List newFacePositions = brush->moveFaces(worldBounds, Polygon3::List(1, face), Vec3(-16.0, -16.0, 0.0));
            ASSERT_EQ(1u, newFacePositions.size());
            ASSERT_TRUE(newFacePositions[0].contains(Vec3(-48.0, -48.0, +32.0)));
            ASSERT_TRUE(newFacePositions[0].contains(Vec3(-48.0, +16.0, +32.0)));
            ASSERT_TRUE(newFacePositions[0].contains(Vec3(+16.0, +16.0, +32.0)));
            ASSERT_TRUE(newFacePositions[0].contains(Vec3(+16.0, -48.0, +32.0)));
            
            newFacePositions = brush->moveFaces(worldBounds, newFacePositions, Vec3(16.0, 16.0, 0.0));
            ASSERT_EQ(1u, newFacePositions.size());
            ASSERT_EQ(4u, newFacePositions[0].vertices().size());
            for (size_t i = 0; i < 4; ++i)
                ASSERT_TRUE(newFacePositions[0].contains(face.vertices()[i]));
            
            delete brush;
        }

        TEST(BrushTest, splitFace) {
            const BBox3 worldBounds(4096.0);
            World world(MapFormat::Standard, NULL, worldBounds);

            BrushBuilder builder(&world, worldBounds);
            Brush* brush = builder.createCube(64.0, "asdf");
            
            Vec3::List vertexPositions(4);
            vertexPositions[0] = Vec3(-32.0, -32.0, +32.0);
            vertexPositions[1] = Vec3(+32.0, -32.0, +32.0);
            vertexPositions[2] = Vec3(+32.0, +32.0, +32.0);
            vertexPositions[3] = Vec3(-32.0, +32.0, +32.0);
            
            const Polygon3 face(vertexPositions);

            const Vec3 newVertexPosition = brush->splitFace(worldBounds, face, Vec3(-16.0, +8.0, +4.0));
            
            ASSERT_VEC_EQ(Vec3(-16.0, +8.0, 36.0), newVertexPosition);
            ASSERT_EQ(9u, brush->vertexCount());
            ASSERT_EQ(16u, brush->edgeCount());
            
            delete brush;
        }
        
    }
}
