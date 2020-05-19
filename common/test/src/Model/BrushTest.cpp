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

#include <catch2/catch.hpp>

#include "TestUtils.h"

#include "FloatType.h"
#include "IO/NodeReader.h"
#include "IO/TestParserStatus.h"
#include "Model/Brush.h"
#include "Model/BrushNode.h"
#include "Model/BrushFace.h"
#include "Model/BrushGeometry.h"
#include "Model/BrushBuilder.h"
#include "Model/Polyhedron.h"
#include "Model/WorldNode.h"

#include <vecmath/polygon.h>
#include <vecmath/ray.h>
#include <vecmath/segment.h>
#include <vecmath/vec.h>

#include <kdl/intrusive_circular_list.h>
#include <kdl/vector_utils.h>

#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        TEST_CASE("BrushTest.constructor_copy", "[BrushTest]") {
            const vm::bbox3 worldBounds(8192.0);
            WorldNode world(MapFormat::Standard);
            const BrushBuilder builder(&world, worldBounds);

            Brush original = builder.createBrush(std::vector<vm::vec3>{vm::vec3(64, -64, 16), vm::vec3(64, 64, 16), vm::vec3(64, -64, -16), vm::vec3(64, 64, -16), vm::vec3(48, 64, 16), vm::vec3(48, 64, -16)}, "texture");
            Brush copy(original);
            
            for (const auto* originalFace : original.faces()) {
                ASSERT_EQ(&original, originalFace->brush());
            }
            
            for (const auto* copyFace : copy.faces()) {
                ASSERT_EQ(&copy, copyFace->brush());
            }
        }
        
        TEST_CASE("BrushTest.constructor_move", "[BrushTest]") {
            const vm::bbox3 worldBounds(8192.0);
            WorldNode world(MapFormat::Standard);
            const BrushBuilder builder(&world, worldBounds);

            Brush original = builder.createBrush(std::vector<vm::vec3>{vm::vec3(64, -64, 16), vm::vec3(64, 64, 16), vm::vec3(64, -64, -16), vm::vec3(64, 64, -16), vm::vec3(48, 64, 16), vm::vec3(48, 64, -16)}, "texture");
            Brush copy(std::move(original));
            
            for (const auto* copyFace : copy.faces()) {
                ASSERT_EQ(&copy, copyFace->brush());
            }
        }
        
        TEST_CASE("BrushTest.operator_assign_copy", "[BrushTest]") {
            const vm::bbox3 worldBounds(8192.0);
            WorldNode world(MapFormat::Standard);
            const BrushBuilder builder(&world, worldBounds);

            Brush original = builder.createBrush(std::vector<vm::vec3>{vm::vec3(64, -64, 16), vm::vec3(64, 64, 16), vm::vec3(64, -64, -16), vm::vec3(64, 64, -16), vm::vec3(48, 64, 16), vm::vec3(48, 64, -16)}, "texture");
            Brush copy;
            copy = original;
            
            for (const auto* originalFace : original.faces()) {
                ASSERT_EQ(&original, originalFace->brush());
            }
            
            for (const auto* copyFace : copy.faces()) {
                ASSERT_EQ(&copy, copyFace->brush());
            }
        }
        
        TEST_CASE("BrushTest.operator_assign_move", "[BrushTest]") {
            const vm::bbox3 worldBounds(8192.0);
            WorldNode world(MapFormat::Standard);
            const BrushBuilder builder(&world, worldBounds);

            Brush original = builder.createBrush(std::vector<vm::vec3>{vm::vec3(64, -64, 16), vm::vec3(64, 64, 16), vm::vec3(64, -64, -16), vm::vec3(64, 64, -16), vm::vec3(48, 64, 16), vm::vec3(48, 64, -16)}, "texture");
            Brush copy;
            copy = std::move(original);
            
            for (const auto* copyFace : copy.faces()) {
                ASSERT_EQ(&copy, copyFace->brush());
            }
        }
        
        TEST_CASE("BrushTest.clip", "[BrushTest]") {
            const vm::bbox3 worldBounds(4096.0);

            // build a cube with length 16 at the origin
            BrushFace* left = BrushFace::createParaxial(vm::vec3(0.0, 0.0, 0.0),
                                                        vm::vec3(0.0, 1.0, 0.0),
                                                        vm::vec3(0.0, 0.0, 1.0));
            BrushFace* right = BrushFace::createParaxial(vm::vec3(16.0, 0.0, 0.0),
                                                         vm::vec3(16.0, 0.0, 1.0),
                                                         vm::vec3(16.0, 1.0, 0.0));
            BrushFace* front = BrushFace::createParaxial(vm::vec3(0.0, 0.0, 0.0),
                                                         vm::vec3(0.0, 0.0, 1.0),
                                                         vm::vec3(1.0, 0.0, 0.0));
            BrushFace* back = BrushFace::createParaxial(vm::vec3(0.0, 16.0, 0.0),
                                                        vm::vec3(1.0, 16.0, 0.0),
                                                        vm::vec3(0.0, 16.0, 1.0));
            BrushFace* top = BrushFace::createParaxial(vm::vec3(0.0, 0.0, 16.0),
                                                       vm::vec3(0.0, 1.0, 16.0),
                                                       vm::vec3(1.0, 0.0, 16.0));
            BrushFace* bottom = BrushFace::createParaxial(vm::vec3(0.0, 0.0, 0.0),
                                                          vm::vec3(1.0, 0.0, 0.0),
                                                          vm::vec3(0.0, 1.0, 0.0));
            BrushFace* clip = BrushFace::createParaxial(vm::vec3(8.0, 0.0, 0.0),
                                                        vm::vec3(8.0, 0.0, 1.0),
                                                        vm::vec3(8.0, 1.0, 0.0));

            std::vector<BrushFace*> faces;
            faces.push_back(left);
            faces.push_back(right);
            faces.push_back(front);
            faces.push_back(back);
            faces.push_back(top);
            faces.push_back(bottom);

            Brush brush(worldBounds, faces);
            ASSERT_TRUE(brush.clip(worldBounds, clip));

            ASSERT_EQ(6u, brush.faces().size());
            ASSERT_TRUE(kdl::vec_contains(brush.faces(), left));
            ASSERT_TRUE(kdl::vec_contains(brush.faces(), clip));
            ASSERT_TRUE(kdl::vec_contains(brush.faces(), front));
            ASSERT_TRUE(kdl::vec_contains(brush.faces(), back));
            ASSERT_TRUE(kdl::vec_contains(brush.faces(), top));
            ASSERT_TRUE(kdl::vec_contains(brush.faces(), bottom));
        }
        
        TEST_CASE("BrushTest.moveBoundary", "[BrushTest]") {
            const vm::bbox3 worldBounds(4096.0);

            // left and right a are slanted!
            BrushFace* left = BrushFace::createParaxial(vm::vec3(0.0, 0.0, 0.0),
                                                        vm::vec3(0.0, 1.0, 0.0),
                                                        vm::vec3(1.0, 0.0, 1.0));
            BrushFace* right = BrushFace::createParaxial(vm::vec3(16.0, 0.0, 0.0),
                                                         vm::vec3(15.0, 0.0, 1.0),
                                                         vm::vec3(16.0, 1.0, 0.0));
            BrushFace* front = BrushFace::createParaxial(vm::vec3(0.0, 0.0, 0.0),
                                                         vm::vec3(0.0, 0.0, 1.0),
                                                         vm::vec3(1.0, 0.0, 0.0));
            BrushFace* back = BrushFace::createParaxial(vm::vec3(0.0, 16.0, 0.0),
                                                        vm::vec3(1.0, 16.0, 0.0),
                                                        vm::vec3(0.0, 16.0, 1.0));
            BrushFace* top = BrushFace::createParaxial(vm::vec3(0.0, 0.0, 6.0),
                                                       vm::vec3(0.0, 1.0, 6.0),
                                                       vm::vec3(1.0, 0.0, 6.0));
            BrushFace* bottom = BrushFace::createParaxial(vm::vec3(0.0, 0.0, 0.0),
                                                          vm::vec3(1.0, 0.0, 0.0),
                                                          vm::vec3(0.0, 1.0, 0.0));
            std::vector<BrushFace*> faces;
            faces.push_back(left);
            faces.push_back(right);
            faces.push_back(front);
            faces.push_back(back);
            faces.push_back(top);
            faces.push_back(bottom);

            Brush brush(worldBounds, faces);
            ASSERT_EQ(6u, brush.faces().size());

            ASSERT_FALSE(brush.canMoveBoundary(worldBounds, top, vm::vec3(0.0, 0.0, +16.0)));
            ASSERT_FALSE(brush.canMoveBoundary(worldBounds, top, vm::vec3(0.0, 0.0, -16.0)));
            ASSERT_FALSE(brush.canMoveBoundary(worldBounds, top, vm::vec3(0.0, 0.0, +2.0)));
            ASSERT_FALSE(brush.canMoveBoundary(worldBounds, top, vm::vec3(0.0, 0.0, -6.0)));
            ASSERT_TRUE(brush.canMoveBoundary(worldBounds, top, vm::vec3(0.0, 0.0, +1.0)));
            ASSERT_TRUE(brush.canMoveBoundary(worldBounds, top, vm::vec3(0.0, 0.0, -5.0)));

            brush.moveBoundary(worldBounds, top, vm::vec3(0.0, 0.0, 1.0), false);
            ASSERT_EQ(6u, brush.faces().size());
            ASSERT_DOUBLE_EQ(7.0, brush.bounds().size().z());
        }

        TEST_CASE("BrushTest.resizePastWorldBounds", "[BrushTest]") {
            const vm::bbox3 worldBounds(8192.0);
            WorldNode world(MapFormat::Standard);
            const BrushBuilder builder(&world, worldBounds);

            Brush brush1 = builder.createBrush(std::vector<vm::vec3>{vm::vec3(64, -64, 16), vm::vec3(64, 64, 16), vm::vec3(64, -64, -16), vm::vec3(64, 64, -16), vm::vec3(48, 64, 16), vm::vec3(48, 64, -16)}, "texture");

            BrushFace* rightFace = brush1.findFace(vm::vec3(1, 0, 0));
            EXPECT_NE(nullptr, rightFace);

            EXPECT_TRUE(brush1.canMoveBoundary(worldBounds, rightFace, vm::vec3(16, 0, 0)));
            EXPECT_FALSE(brush1.canMoveBoundary(worldBounds, rightFace, vm::vec3(8000, 0, 0)));
        }

        TEST_CASE("BrushTest.expand", "[BrushTest]") {
            const vm::bbox3 worldBounds(8192.0);
            WorldNode world(MapFormat::Standard);
            const BrushBuilder builder(&world, worldBounds);

            Brush brush1 = builder.createCuboid(vm::bbox3(vm::vec3(-64, -64, -64), vm::vec3(64, 64, 64)), "texture");
            EXPECT_TRUE(brush1.canExpand(worldBounds, 6, true));
            EXPECT_TRUE(brush1.expand(worldBounds, 6, true));

            const vm::bbox3 expandedBBox(vm::vec3(-70, -70, -70), vm::vec3(70, 70, 70));

            EXPECT_EQ(expandedBBox, brush1.bounds());
            EXPECT_COLLECTIONS_EQUIVALENT(expandedBBox.vertices(), brush1.vertexPositions());
        }

        TEST_CASE("BrushTest.contract", "[BrushTest]") {
            const vm::bbox3 worldBounds(8192.0);
            WorldNode world(MapFormat::Standard);
            const BrushBuilder builder(&world, worldBounds);

            Brush brush1 = builder.createCuboid(vm::bbox3(vm::vec3(-64, -64, -64), vm::vec3(64, 64, 64)), "texture");
            EXPECT_TRUE(brush1.canExpand(worldBounds, -32, true));
            EXPECT_TRUE(brush1.expand(worldBounds, -32, true));

            const vm::bbox3 expandedBBox(vm::vec3(-32, -32, -32), vm::vec3(32, 32, 32));

            EXPECT_EQ(expandedBBox, brush1.bounds());
            EXPECT_COLLECTIONS_EQUIVALENT(expandedBBox.vertices(), brush1.vertexPositions());
        }

        TEST_CASE("BrushTest.contractToZero", "[BrushTest]") {
            const vm::bbox3 worldBounds(8192.0);
            WorldNode world(MapFormat::Standard);
            const BrushBuilder builder(&world, worldBounds);

            Brush brush1 = builder.createCuboid(vm::bbox3(vm::vec3(-64, -64, -64), vm::vec3(64, 64, 64)), "texture");
            EXPECT_FALSE(brush1.canExpand(worldBounds, -64, true));
            EXPECT_FALSE(brush1.expand(worldBounds, -64, true));
        }

        TEST_CASE("BrushTest.moveVertex", "[BrushTest]") {
            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(MapFormat::Standard);

            BrushBuilder builder(&world, worldBounds);
            Brush brush = builder.createCube(64.0, "left", "right", "front", "back", "top", "bottom");

            const vm::vec3 p1(-32.0, -32.0, -32.0);
            const vm::vec3 p2(-32.0, -32.0, +32.0);
            const vm::vec3 p3(-32.0, +32.0, -32.0);
            const vm::vec3 p4(-32.0, +32.0, +32.0);
            const vm::vec3 p5(+32.0, -32.0, -32.0);
            const vm::vec3 p6(+32.0, -32.0, +32.0);
            const vm::vec3 p7(+32.0, +32.0, -32.0);
            const vm::vec3 p8(+32.0, +32.0, +32.0);
            const vm::vec3 p9(+16.0, +16.0, +32.0);

            std::vector<vm::vec3> newVertexPositions = brush.moveVertices(worldBounds, std::vector<vm::vec3>(1, p8), p9 - p8);
            ASSERT_EQ(1u, newVertexPositions.size());
            ASSERT_VEC_EQ(p9, newVertexPositions[0]);

            assertTexture("left", brush, p1, p2, p4, p3);
            assertTexture("right", brush, p5, p7, p6);
            assertTexture("right", brush, p6, p7, p9);
            assertTexture("front", brush, p1, p5, p6, p2);
            assertTexture("back", brush, p3, p4, p7);
            assertTexture("back", brush, p4, p9, p7);
            assertTexture("top", brush, p2, p6, p9, p4);
            assertTexture("bottom", brush, p1, p3, p7, p5);

            newVertexPositions = brush.moveVertices(worldBounds, newVertexPositions, p8 - p9);
            ASSERT_EQ(1u, newVertexPositions.size());
            ASSERT_VEC_EQ(p8, newVertexPositions[0]);

            assertTexture("left", brush, p1, p2, p4, p3);
            assertTexture("right", brush, p5, p7, p8, p6);
            assertTexture("front", brush, p1, p5, p6, p2);
            assertTexture("back", brush, p3, p4, p8, p7);
            assertTexture("top", brush, p2, p6, p8, p4);
            assertTexture("bottom", brush, p1, p3, p7, p5);
        }

        TEST_CASE("BrushTest.moveTetrahedronVertexToOpposideSide", "[BrushTest]") {
            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(MapFormat::Standard);

            const vm::vec3 top(0.0, 0.0, +16.0);

            std::vector<vm::vec3> points;
            points.push_back(vm::vec3(-16.0, -16.0, 0.0));
            points.push_back(vm::vec3(+16.0, -16.0, 0.0));
            points.push_back(vm::vec3(0.0, +16.0, 0.0));
            points.push_back(top);

            BrushBuilder builder(&world, worldBounds);
            Brush brush = builder.createBrush(points, "some_texture");

            std::vector<vm::vec3> newVertexPositions = brush.moveVertices(worldBounds, std::vector<vm::vec3>(1, top), vm::vec3(0.0, 0.0, -32.0));
            ASSERT_EQ(1u, newVertexPositions.size());
            ASSERT_VEC_EQ(vm::vec3(0.0, 0.0, -16.0), newVertexPositions[0]);
            ASSERT_TRUE(brush.fullySpecified());
        }

        TEST_CASE("BrushTest.moveVertexInwardWithoutMerges", "[BrushTest]") {
            const vm::vec3d p1(-64.0, -64.0, -64.0);
            const vm::vec3d p2(-64.0, -64.0, +64.0);
            const vm::vec3d p3(-64.0, +64.0, -64.0);
            const vm::vec3d p4(-64.0, +64.0, +64.0);
            const vm::vec3d p5(+64.0, -64.0, -64.0);
            const vm::vec3d p6(+64.0, -64.0, +64.0);
            const vm::vec3d p7(+64.0, +64.0, -64.0);
            const vm::vec3d p8(+64.0, +64.0, +64.0);
            const vm::vec3d p9(+56.0, +56.0, +56.0);

            std::vector<vm::vec3d> oldPositions;
            oldPositions.push_back(p1);
            oldPositions.push_back(p2);
            oldPositions.push_back(p3);
            oldPositions.push_back(p4);
            oldPositions.push_back(p5);
            oldPositions.push_back(p6);
            oldPositions.push_back(p7);
            oldPositions.push_back(p8);

            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(MapFormat::Standard);

            BrushBuilder builder(&world, worldBounds);
            Brush brush = builder.createBrush(oldPositions, "texture");

            const std::vector<vm::vec3d> result = brush.moveVertices(worldBounds, std::vector<vm::vec3d>(1, p8), p9 - p8);
            ASSERT_EQ(1u, result.size());
            ASSERT_VEC_EQ(p9, result[0]);

            ASSERT_EQ(8u, brush.vertexCount());
            ASSERT_EQ(15u, brush.edgeCount());
            ASSERT_EQ(9u, brush.faceCount());

            ASSERT_TRUE(brush.hasVertex(p1));
            ASSERT_TRUE(brush.hasVertex(p2));
            ASSERT_TRUE(brush.hasVertex(p3));
            ASSERT_TRUE(brush.hasVertex(p4));
            ASSERT_TRUE(brush.hasVertex(p5));
            ASSERT_TRUE(brush.hasVertex(p6));
            ASSERT_TRUE(brush.hasVertex(p7));
            ASSERT_TRUE(brush.hasVertex(p9));

            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p2)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p3)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p5)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p2, p4)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p2, p6)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p3, p4)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p3, p7)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p4, p6)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p4, p7)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p4, p9)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p5, p6)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p5, p7)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p6, p7)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p6, p9)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p7, p9)));


            ASSERT_TRUE(brush.hasFace(p1, p5, p6, p2));
            ASSERT_TRUE(brush.hasFace(p1, p2, p4, p3));
            ASSERT_TRUE(brush.hasFace(p1, p3, p7, p5));
            ASSERT_TRUE(brush.hasFace(p2, p6, p4));
            ASSERT_TRUE(brush.hasFace(p5, p7, p6));
            ASSERT_TRUE(brush.hasFace(p3, p4, p7));
            ASSERT_TRUE(brush.hasFace(p9, p6, p7));
            ASSERT_TRUE(brush.hasFace(p9, p4, p6));
            ASSERT_TRUE(brush.hasFace(p9, p7, p4));
        }

        TEST_CASE("BrushTest.moveVertexOutwardWithoutMerges", "[BrushTest]") {
            const vm::vec3d p1(-64.0, -64.0, -64.0);
            const vm::vec3d p2(-64.0, -64.0, +64.0);
            const vm::vec3d p3(-64.0, +64.0, -64.0);
            const vm::vec3d p4(-64.0, +64.0, +64.0);
            const vm::vec3d p5(+64.0, -64.0, -64.0);
            const vm::vec3d p6(+64.0, -64.0, +64.0);
            const vm::vec3d p7(+64.0, +64.0, -64.0);
            const vm::vec3d p8(+64.0, +64.0, +64.0);
            const vm::vec3d p9(+72.0, +72.0, +72.0);

            std::vector<vm::vec3d> oldPositions;
            oldPositions.push_back(p1);
            oldPositions.push_back(p2);
            oldPositions.push_back(p3);
            oldPositions.push_back(p4);
            oldPositions.push_back(p5);
            oldPositions.push_back(p6);
            oldPositions.push_back(p7);
            oldPositions.push_back(p8);

            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(MapFormat::Standard);

            BrushBuilder builder(&world, worldBounds);
            Brush brush = builder.createBrush(oldPositions, "texture");

            const std::vector<vm::vec3d> result = brush.moveVertices(worldBounds, std::vector<vm::vec3d>(1, p8), p9 - p8);
            ASSERT_EQ(1u, result.size());
            ASSERT_VEC_EQ(p9, result[0]);

            ASSERT_EQ(8u, brush.vertexCount());
            ASSERT_EQ(15u, brush.edgeCount());
            ASSERT_EQ(9u, brush.faceCount());

            ASSERT_TRUE(brush.hasVertex(p1));
            ASSERT_TRUE(brush.hasVertex(p2));
            ASSERT_TRUE(brush.hasVertex(p3));
            ASSERT_TRUE(brush.hasVertex(p4));
            ASSERT_TRUE(brush.hasVertex(p5));
            ASSERT_TRUE(brush.hasVertex(p6));
            ASSERT_TRUE(brush.hasVertex(p7));
            ASSERT_TRUE(brush.hasVertex(p9));

            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p2)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p3)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p5)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p2, p4)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p2, p6)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p2, p9)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p3, p4)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p3, p7)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p3, p9)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p4, p9)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p5, p6)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p5, p7)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p5, p9)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p6, p9)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p7, p9)));

            ASSERT_TRUE(brush.hasFace(p1, p5, p6, p2));
            ASSERT_TRUE(brush.hasFace(p1, p2, p4, p3));
            ASSERT_TRUE(brush.hasFace(p1, p3, p7, p5));
            ASSERT_TRUE(brush.hasFace(p2, p6, p9));
            ASSERT_TRUE(brush.hasFace(p2, p9, p4));
            ASSERT_TRUE(brush.hasFace(p3, p4, p9));
            ASSERT_TRUE(brush.hasFace(p3, p9, p7));
            ASSERT_TRUE(brush.hasFace(p5, p9, p6));
            ASSERT_TRUE(brush.hasFace(p5, p7, p9));
        }

        TEST_CASE("BrushTest.moveVertexWithOneOuterNeighbourMerge", "[BrushTest]") {
            const vm::vec3d p1(-64.0, -64.0, -64.0);
            const vm::vec3d p2(-64.0, -64.0, +64.0);
            const vm::vec3d p3(-64.0, +64.0, -64.0);
            const vm::vec3d p4(-64.0, +64.0, +64.0);
            const vm::vec3d p5(+64.0, -64.0, -64.0);
            const vm::vec3d p6(+64.0, -64.0, +64.0);
            const vm::vec3d p7(+64.0, +64.0, -64.0);
            const vm::vec3d p8(+56.0, +56.0, +56.0);
            const vm::vec3d p9(+56.0, +56.0, +64.0);

            std::vector<vm::vec3d> oldPositions;
            oldPositions.push_back(p1);
            oldPositions.push_back(p2);
            oldPositions.push_back(p3);
            oldPositions.push_back(p4);
            oldPositions.push_back(p5);
            oldPositions.push_back(p6);
            oldPositions.push_back(p7);
            oldPositions.push_back(p8);

            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(MapFormat::Standard);

            BrushBuilder builder(&world, worldBounds);
            Brush brush = builder.createBrush(oldPositions, "texture");

            const std::vector<vm::vec3d> result = brush.moveVertices(worldBounds, std::vector<vm::vec3d>(1, p8), p9 - p8);
            ASSERT_EQ(1u, result.size());
            ASSERT_VEC_EQ(p9, result[0]);

            ASSERT_EQ(8u, brush.vertexCount());
            ASSERT_EQ(14u, brush.edgeCount());
            ASSERT_EQ(8u, brush.faceCount());

            ASSERT_TRUE(brush.hasVertex(p1));
            ASSERT_TRUE(brush.hasVertex(p2));
            ASSERT_TRUE(brush.hasVertex(p3));
            ASSERT_TRUE(brush.hasVertex(p4));
            ASSERT_TRUE(brush.hasVertex(p5));
            ASSERT_TRUE(brush.hasVertex(p6));
            ASSERT_TRUE(brush.hasVertex(p7));
            ASSERT_TRUE(brush.hasVertex(p9));

            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p2)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p3)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p5)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p2, p4)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p2, p6)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p3, p4)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p3, p7)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p4, p7)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p4, p9)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p5, p6)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p5, p7)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p6, p7)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p6, p9)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p7, p9)));

            ASSERT_TRUE(brush.hasFace(p1, p5, p6, p2));
            ASSERT_TRUE(brush.hasFace(p1, p2, p4, p3));
            ASSERT_TRUE(brush.hasFace(p1, p3, p7, p5));
            ASSERT_TRUE(brush.hasFace(p2, p6, p9, p4));
            ASSERT_TRUE(brush.hasFace(p5, p7, p6));
            ASSERT_TRUE(brush.hasFace(p3, p4, p7));
            ASSERT_TRUE(brush.hasFace(p9, p6, p7));
            ASSERT_TRUE(brush.hasFace(p9, p7, p4));
        }

        TEST_CASE("BrushTest.moveVertexWithTwoOuterNeighbourMerges", "[BrushTest]") {
            const vm::vec3d p1(-64.0, -64.0, -64.0);
            const vm::vec3d p2(-64.0, -64.0, +64.0);
            const vm::vec3d p3(-64.0, +64.0, -64.0);
            const vm::vec3d p4(-64.0, +64.0, +64.0);
            const vm::vec3d p5(+64.0, -64.0, -64.0);
            const vm::vec3d p6(+64.0, -64.0, +64.0);
            const vm::vec3d p7(+64.0, +64.0, -64.0);
            const vm::vec3d p8(+56.0, +56.0, +56.0);
            const vm::vec3d p9(+64.0, +64.0, +56.0);

            std::vector<vm::vec3d> oldPositions;
            oldPositions.push_back(p1);
            oldPositions.push_back(p2);
            oldPositions.push_back(p3);
            oldPositions.push_back(p4);
            oldPositions.push_back(p5);
            oldPositions.push_back(p6);
            oldPositions.push_back(p7);
            oldPositions.push_back(p8);

            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(MapFormat::Standard);

            BrushBuilder builder(&world, worldBounds);
            Brush brush = builder.createBrush(oldPositions, "texture");

            const std::vector<vm::vec3d> result = brush.moveVertices(worldBounds, std::vector<vm::vec3d>(1, p8), p9 - p8);
            ASSERT_EQ(1u, result.size());
            ASSERT_VEC_EQ(p9, result[0]);

            ASSERT_EQ(8u, brush.vertexCount());
            ASSERT_EQ(13u, brush.edgeCount());
            ASSERT_EQ(7u, brush.faceCount());

            ASSERT_TRUE(brush.hasVertex(p1));
            ASSERT_TRUE(brush.hasVertex(p2));
            ASSERT_TRUE(brush.hasVertex(p3));
            ASSERT_TRUE(brush.hasVertex(p4));
            ASSERT_TRUE(brush.hasVertex(p5));
            ASSERT_TRUE(brush.hasVertex(p6));
            ASSERT_TRUE(brush.hasVertex(p7));
            ASSERT_TRUE(brush.hasVertex(p9));

            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p2)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p3)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p5)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p2, p4)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p2, p6)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p3, p4)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p3, p7)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p4, p6)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p4, p9)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p5, p6)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p5, p7)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p6, p9)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p7, p9)));

            ASSERT_TRUE(brush.hasFace(p1, p5, p6, p2));
            ASSERT_TRUE(brush.hasFace(p1, p2, p4, p3));
            ASSERT_TRUE(brush.hasFace(p1, p3, p7, p5));
            ASSERT_TRUE(brush.hasFace(p5, p7, p9, p6));
            ASSERT_TRUE(brush.hasFace(p3, p4, p9, p7));
            ASSERT_TRUE(brush.hasFace(p2, p6, p4));
            ASSERT_TRUE(brush.hasFace(p9, p4, p6));
        }

        TEST_CASE("BrushTest.moveVertexWithAllOuterNeighbourMerges", "[BrushTest]") {
            const vm::vec3d p1(-64.0, -64.0, -64.0);
            const vm::vec3d p2(-64.0, -64.0, +64.0);
            const vm::vec3d p3(-64.0, +64.0, -64.0);
            const vm::vec3d p4(-64.0, +64.0, +64.0);
            const vm::vec3d p5(+64.0, -64.0, -64.0);
            const vm::vec3d p6(+64.0, -64.0, +64.0);
            const vm::vec3d p7(+64.0, +64.0, -64.0);
            const vm::vec3d p8(+56.0, +56.0, +56.0);
            const vm::vec3d p9(+64.0, +64.0, +64.0);

            std::vector<vm::vec3d> oldPositions;
            oldPositions.push_back(p1);
            oldPositions.push_back(p2);
            oldPositions.push_back(p3);
            oldPositions.push_back(p4);
            oldPositions.push_back(p5);
            oldPositions.push_back(p6);
            oldPositions.push_back(p7);
            oldPositions.push_back(p8);

            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(MapFormat::Standard);

            BrushBuilder builder(&world, worldBounds);
            Brush brush = builder.createBrush(oldPositions, "texture");

            const std::vector<vm::vec3d> result = brush.moveVertices(worldBounds, std::vector<vm::vec3d>(1, p8), p9 - p8);
            ASSERT_EQ(1u, result.size());
            ASSERT_VEC_EQ(p9, result[0]);

            ASSERT_EQ(8u, brush.vertexCount());
            ASSERT_EQ(12u, brush.edgeCount());
            ASSERT_EQ(6u, brush.faceCount());

            ASSERT_TRUE(brush.hasVertex(p1));
            ASSERT_TRUE(brush.hasVertex(p2));
            ASSERT_TRUE(brush.hasVertex(p3));
            ASSERT_TRUE(brush.hasVertex(p4));
            ASSERT_TRUE(brush.hasVertex(p5));
            ASSERT_TRUE(brush.hasVertex(p6));
            ASSERT_TRUE(brush.hasVertex(p7));
            ASSERT_TRUE(brush.hasVertex(p9));

            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p2)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p3)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p5)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p2, p4)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p2, p6)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p3, p4)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p3, p7)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p4, p9)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p5, p6)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p5, p7)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p6, p9)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p7, p9)));

            ASSERT_TRUE(brush.hasFace(p1, p5, p6, p2));
            ASSERT_TRUE(brush.hasFace(p1, p2, p4, p3));
            ASSERT_TRUE(brush.hasFace(p1, p3, p7, p5));
            ASSERT_TRUE(brush.hasFace(p2, p6, p9, p4));
            ASSERT_TRUE(brush.hasFace(p3, p4, p9, p7));
            ASSERT_TRUE(brush.hasFace(p5, p7, p9, p6));
        }

        TEST_CASE("BrushTest.moveVertexWithAllInnerNeighbourMerge", "[BrushTest]") {
            const vm::vec3d p1(-64.0, -64.0, -64.0);
            const vm::vec3d p2(-64.0, -64.0, +64.0);
            const vm::vec3d p3(-64.0, +64.0, -64.0);
            const vm::vec3d p4(-64.0, +64.0, +64.0);
            const vm::vec3d p5(+64.0, -64.0, -64.0);
            const vm::vec3d p6(+64.0, -64.0, +64.0);
            const vm::vec3d p7(+64.0, +64.0, -64.0);
            const vm::vec3d p8(+64.0, +64.0, +64.0);
            const vm::vec3d p9(0.0, 0.0, 0.0);

            std::vector<vm::vec3d> oldPositions;
            oldPositions.push_back(p1);
            oldPositions.push_back(p2);
            oldPositions.push_back(p3);
            oldPositions.push_back(p4);
            oldPositions.push_back(p5);
            oldPositions.push_back(p6);
            oldPositions.push_back(p7);
            oldPositions.push_back(p8);

            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(MapFormat::Standard);

            BrushBuilder builder(&world, worldBounds);
            Brush brush = builder.createBrush(oldPositions, "texture");

            const std::vector<vm::vec3d> result = brush.moveVertices(worldBounds, std::vector<vm::vec3d>(1, p8), p9 - p8);
            ASSERT_EQ(0u, result.size());

            ASSERT_EQ(7u, brush.vertexCount());
            ASSERT_EQ(12u, brush.edgeCount());
            ASSERT_EQ(7u, brush.faceCount());

            ASSERT_TRUE(brush.hasVertex(p1));
            ASSERT_TRUE(brush.hasVertex(p2));
            ASSERT_TRUE(brush.hasVertex(p3));
            ASSERT_TRUE(brush.hasVertex(p4));
            ASSERT_TRUE(brush.hasVertex(p5));
            ASSERT_TRUE(brush.hasVertex(p6));
            ASSERT_TRUE(brush.hasVertex(p7));

            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p2)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p3)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p5)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p2, p4)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p2, p6)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p3, p4)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p3, p7)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p4, p6)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p4, p7)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p5, p6)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p5, p7)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p6, p7)));

            ASSERT_TRUE(brush.hasFace(p1, p5, p6, p2));
            ASSERT_TRUE(brush.hasFace(p1, p2, p4, p3));
            ASSERT_TRUE(brush.hasFace(p1, p3, p7, p5));
            ASSERT_TRUE(brush.hasFace(p2, p6, p4));
            ASSERT_TRUE(brush.hasFace(p3, p4, p7));
            ASSERT_TRUE(brush.hasFace(p5, p7, p6));
            ASSERT_TRUE(brush.hasFace(p4, p6, p7));
        }

        TEST_CASE("BrushTest.moveVertexUpThroughPlane", "[BrushTest]") {
            const vm::vec3d p1(-64.0, -64.0, -64.0);
            const vm::vec3d p2(-64.0, -64.0, +64.0);
            const vm::vec3d p3(-64.0, +64.0, -64.0);
            const vm::vec3d p4(-64.0, +64.0, +64.0);
            const vm::vec3d p5(+64.0, -64.0, -64.0);
            const vm::vec3d p6(+64.0, -64.0, +64.0);
            const vm::vec3d p7(+64.0, +64.0, -64.0);
            const vm::vec3d p8(+64.0, +64.0, +56.0);
            const vm::vec3d p9(+64.0, +64.0, +72.0);

            std::vector<vm::vec3d> oldPositions;
            oldPositions.push_back(p1);
            oldPositions.push_back(p2);
            oldPositions.push_back(p3);
            oldPositions.push_back(p4);
            oldPositions.push_back(p5);
            oldPositions.push_back(p6);
            oldPositions.push_back(p7);
            oldPositions.push_back(p8);

            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(MapFormat::Standard);

            BrushBuilder builder(&world, worldBounds);
            Brush brush = builder.createBrush(oldPositions, "texture");

            const std::vector<vm::vec3d> result = brush.moveVertices(worldBounds, std::vector<vm::vec3d>(1, p8), p9 - p8);
            ASSERT_EQ(1u, result.size());
            ASSERT_VEC_EQ(p9, result[0]);

            ASSERT_EQ(8u, brush.vertexCount());
            ASSERT_EQ(13u, brush.edgeCount());
            ASSERT_EQ(7u, brush.faceCount());

            ASSERT_TRUE(brush.hasVertex(p1));
            ASSERT_TRUE(brush.hasVertex(p2));
            ASSERT_TRUE(brush.hasVertex(p3));
            ASSERT_TRUE(brush.hasVertex(p4));
            ASSERT_TRUE(brush.hasVertex(p5));
            ASSERT_TRUE(brush.hasVertex(p6));
            ASSERT_TRUE(brush.hasVertex(p7));
            ASSERT_TRUE(brush.hasVertex(p9));

            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p2)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p3)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p5)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p2, p4)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p2, p6)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p2, p9)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p3, p4)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p3, p7)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p4, p9)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p5, p6)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p5, p7)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p6, p9)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p7, p9)));

            ASSERT_TRUE(brush.hasFace(p1, p5, p6, p2));
            ASSERT_TRUE(brush.hasFace(p1, p2, p4, p3));
            ASSERT_TRUE(brush.hasFace(p1, p3, p7, p5));
            ASSERT_TRUE(brush.hasFace(p3, p4, p9, p7));
            ASSERT_TRUE(brush.hasFace(p5, p7, p9, p6));
            ASSERT_TRUE(brush.hasFace(p2, p9, p4));
            ASSERT_TRUE(brush.hasFace(p2, p6, p9));
        }

        TEST_CASE("BrushTest.moveVertexOntoEdge", "[BrushTest]") {
            const vm::vec3d p1(-64.0, -64.0, -64.0);
            const vm::vec3d p2(-64.0, -64.0, +64.0);
            const vm::vec3d p3(-64.0, +64.0, -64.0);
            const vm::vec3d p4(-64.0, +64.0, +64.0);
            const vm::vec3d p5(+64.0, -64.0, -64.0);
            const vm::vec3d p6(+64.0, -64.0, +64.0);
            const vm::vec3d p7(+64.0, +64.0, -64.0);
            const vm::vec3d p8(+64.0, +64.0, 0.0);
            const vm::vec3d p9(0.0, 0.0, +64.0);

            std::vector<vm::vec3d> oldPositions;
            oldPositions.push_back(p1);
            oldPositions.push_back(p2);
            oldPositions.push_back(p3);
            oldPositions.push_back(p4);
            oldPositions.push_back(p5);
            oldPositions.push_back(p6);
            oldPositions.push_back(p7);
            oldPositions.push_back(p8);

            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(MapFormat::Standard);

            BrushBuilder builder(&world, worldBounds);
            Brush brush = builder.createBrush(oldPositions, "texture");

            const std::vector<vm::vec3d> result = brush.moveVertices(worldBounds, std::vector<vm::vec3d>(1, p8), p9 - p8);
            ASSERT_EQ(0u, result.size());

            ASSERT_EQ(7u, brush.vertexCount());
            ASSERT_EQ(12u, brush.edgeCount());
            ASSERT_EQ(7u, brush.faceCount());

            ASSERT_TRUE(brush.hasVertex(p1));
            ASSERT_TRUE(brush.hasVertex(p2));
            ASSERT_TRUE(brush.hasVertex(p3));
            ASSERT_TRUE(brush.hasVertex(p4));
            ASSERT_TRUE(brush.hasVertex(p5));
            ASSERT_TRUE(brush.hasVertex(p6));
            ASSERT_TRUE(brush.hasVertex(p7));

            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p2)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p3)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p5)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p2, p4)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p2, p6)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p3, p4)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p3, p7)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p4, p6)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p4, p7)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p5, p6)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p5, p7)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p6, p7)));

            ASSERT_TRUE(brush.hasFace(p1, p5, p6, p2));
            ASSERT_TRUE(brush.hasFace(p1, p2, p4, p3));
            ASSERT_TRUE(brush.hasFace(p1, p3, p7, p5));
            ASSERT_TRUE(brush.hasFace(p2, p6, p4));
            ASSERT_TRUE(brush.hasFace(p3, p4, p7));
            ASSERT_TRUE(brush.hasFace(p5, p7, p6));
            ASSERT_TRUE(brush.hasFace(p4, p6, p7));
        }

        TEST_CASE("BrushTest.moveVertexOntoIncidentVertex", "[BrushTest]") {
            const vm::vec3d p1(-64.0, -64.0, -64.0);
            const vm::vec3d p2(-64.0, -64.0, +64.0);
            const vm::vec3d p3(-64.0, +64.0, -64.0);
            const vm::vec3d p4(-64.0, +64.0, +64.0);
            const vm::vec3d p5(+64.0, -64.0, -64.0);
            const vm::vec3d p6(+64.0, -64.0, +64.0);
            const vm::vec3d p7(+64.0, +64.0, -64.0);
            const vm::vec3d p8(+64.0, +64.0, +64.0);

            std::vector<vm::vec3d> oldPositions;
            oldPositions.push_back(p1);
            oldPositions.push_back(p2);
            oldPositions.push_back(p3);
            oldPositions.push_back(p4);
            oldPositions.push_back(p5);
            oldPositions.push_back(p6);
            oldPositions.push_back(p7);
            oldPositions.push_back(p8);

            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(MapFormat::Standard);

            BrushBuilder builder(&world, worldBounds);
            Brush brush = builder.createBrush(oldPositions, "texture");

            const std::vector<vm::vec3d> result = brush.moveVertices(worldBounds, std::vector<vm::vec3d>(1, p8), p7 - p8);
            ASSERT_EQ(1u, result.size());
            ASSERT_VEC_EQ(p7, result[0]);

            ASSERT_EQ(7u, brush.vertexCount());
            ASSERT_EQ(12u, brush.edgeCount());
            ASSERT_EQ(7u, brush.faceCount());

            ASSERT_TRUE(brush.hasVertex(p1));
            ASSERT_TRUE(brush.hasVertex(p2));
            ASSERT_TRUE(brush.hasVertex(p3));
            ASSERT_TRUE(brush.hasVertex(p4));
            ASSERT_TRUE(brush.hasVertex(p5));
            ASSERT_TRUE(brush.hasVertex(p6));
            ASSERT_TRUE(brush.hasVertex(p7));

            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p2)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p3)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p5)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p2, p4)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p2, p6)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p3, p4)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p3, p7)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p4, p6)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p4, p7)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p5, p6)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p5, p7)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p6, p7)));

            ASSERT_TRUE(brush.hasFace(p1, p5, p6, p2));
            ASSERT_TRUE(brush.hasFace(p1, p2, p4, p3));
            ASSERT_TRUE(brush.hasFace(p1, p3, p7, p5));
            ASSERT_TRUE(brush.hasFace(p2, p6, p4));
            ASSERT_TRUE(brush.hasFace(p3, p4, p7));
            ASSERT_TRUE(brush.hasFace(p5, p7, p6));
            ASSERT_TRUE(brush.hasFace(p4, p6, p7));
        }

        TEST_CASE("BrushTest.moveVertexOntoIncidentVertexInOppositeDirection", "[BrushTest]") {
            const vm::vec3d p1(-64.0, -64.0, -64.0);
            const vm::vec3d p2(-64.0, -64.0, +64.0);
            const vm::vec3d p3(-64.0, +64.0, -64.0);
            const vm::vec3d p4(-64.0, +64.0, +64.0);
            const vm::vec3d p5(+64.0, -64.0, -64.0);
            const vm::vec3d p6(+64.0, -64.0, +64.0);
            const vm::vec3d p7(+64.0, +64.0, -64.0);
            const vm::vec3d p8(+64.0, +64.0, +64.0);

            std::vector<vm::vec3d> oldPositions;
            oldPositions.push_back(p1);
            oldPositions.push_back(p2);
            oldPositions.push_back(p3);
            oldPositions.push_back(p4);
            oldPositions.push_back(p5);
            oldPositions.push_back(p6);
            oldPositions.push_back(p7);
            oldPositions.push_back(p8);

            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(MapFormat::Standard);

            BrushBuilder builder(&world, worldBounds);
            Brush brush = builder.createBrush(oldPositions, "texture");

            const std::vector<vm::vec3d> result = brush.moveVertices(worldBounds, std::vector<vm::vec3d>(1, p7), p8 - p7);
            ASSERT_EQ(1u, result.size());
            ASSERT_VEC_EQ(p8, result[0]);

            ASSERT_EQ(7u, brush.vertexCount());
            ASSERT_EQ(12u, brush.edgeCount());
            ASSERT_EQ(7u, brush.faceCount());

            ASSERT_TRUE(brush.hasVertex(p1));
            ASSERT_TRUE(brush.hasVertex(p2));
            ASSERT_TRUE(brush.hasVertex(p3));
            ASSERT_TRUE(brush.hasVertex(p4));
            ASSERT_TRUE(brush.hasVertex(p5));
            ASSERT_TRUE(brush.hasVertex(p6));
            ASSERT_TRUE(brush.hasVertex(p8));

            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p2)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p3)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p5)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p2, p4)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p2, p6)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p3, p4)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p3, p5)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p3, p8)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p4, p8)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p5, p6)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p5, p8)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p6, p8)));

            ASSERT_TRUE(brush.hasFace(p1, p5, p6, p2));
            ASSERT_TRUE(brush.hasFace(p1, p2, p4, p3));
            ASSERT_TRUE(brush.hasFace(p2, p6, p8, p4));
            ASSERT_TRUE(brush.hasFace(p1, p3, p5));
            ASSERT_TRUE(brush.hasFace(p3, p4, p8));
            ASSERT_TRUE(brush.hasFace(p5, p8, p6));
            ASSERT_TRUE(brush.hasFace(p3, p8, p5));
        }

        TEST_CASE("BrushTest.moveVertexAndMergeColinearEdgesWithoutDeletingVertex", "[BrushTest]") {
            const vm::vec3d p1(-64.0, -64.0, -64.0);
            const vm::vec3d p2(-64.0, -64.0, +64.0);
            const vm::vec3d p3(-64.0, +64.0, -64.0);
            const vm::vec3d p4(-64.0, +64.0, +64.0);
            const vm::vec3d p5(+64.0, -64.0, -64.0);
            const vm::vec3d p6(+64.0, -64.0, +64.0);
            const vm::vec3d p7(+64.0, +64.0, -64.0);
            const vm::vec3d p8(+64.0, +64.0, +64.0);
            const vm::vec3d p9(+80.0, +64.0, +64.0);

            std::vector<vm::vec3d> oldPositions;
            oldPositions.push_back(p1);
            oldPositions.push_back(p2);
            oldPositions.push_back(p3);
            oldPositions.push_back(p4);
            oldPositions.push_back(p5);
            oldPositions.push_back(p6);
            oldPositions.push_back(p7);
            oldPositions.push_back(p8);

            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(MapFormat::Standard);

            BrushBuilder builder(&world, worldBounds);
            Brush brush = builder.createBrush(oldPositions, "texture");

            const std::vector<vm::vec3d> result = brush.moveVertices(worldBounds, std::vector<vm::vec3d>(1, p6), p9 - p6);
            ASSERT_EQ(1u, result.size());
            ASSERT_VEC_EQ(p9, result[0]);

            ASSERT_EQ(7u, brush.vertexCount());
            ASSERT_EQ(12u, brush.edgeCount());
            ASSERT_EQ(7u, brush.faceCount());

            ASSERT_TRUE(brush.hasVertex(p1));
            ASSERT_TRUE(brush.hasVertex(p2));
            ASSERT_TRUE(brush.hasVertex(p3));
            ASSERT_TRUE(brush.hasVertex(p4));
            ASSERT_TRUE(brush.hasVertex(p5));
            ASSERT_TRUE(brush.hasVertex(p7));
            ASSERT_TRUE(brush.hasVertex(p9));

            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p2)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p3)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p5)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p2, p4)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p2, p5)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p2, p9)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p3, p4)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p3, p7)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p4, p9)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p5, p7)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p5, p9)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p7, p9)));

            ASSERT_TRUE(brush.hasFace(p1, p2, p4, p3));
            ASSERT_TRUE(brush.hasFace(p1, p3, p7, p5));
            ASSERT_TRUE(brush.hasFace(p3, p4, p9, p7));
            ASSERT_TRUE(brush.hasFace(p1, p5, p2));
            ASSERT_TRUE(brush.hasFace(p2, p5, p9));
            ASSERT_TRUE(brush.hasFace(p2, p9, p4));
            ASSERT_TRUE(brush.hasFace(p5, p7, p9));
        }

        TEST_CASE("BrushTest.moveVertexAndMergeColinearEdgesWithoutDeletingVertex2", "[BrushTest]") {
            const vm::vec3d p1(-64.0, -64.0, -64.0);
            const vm::vec3d p2(-64.0, -64.0, +64.0);
            const vm::vec3d p3(-64.0, +64.0, -64.0);
            const vm::vec3d p4(-64.0, +64.0, +64.0);
            const vm::vec3d p5(+64.0, -64.0, -64.0);
            const vm::vec3d p6(+64.0, -64.0, +64.0);
            const vm::vec3d p7(+64.0, +64.0, -64.0);
            const vm::vec3d p8(+64.0, +64.0, +64.0);
            const vm::vec3d p9(+80.0, -64.0, +64.0);

            std::vector<vm::vec3d> oldPositions;
            oldPositions.push_back(p1);
            oldPositions.push_back(p2);
            oldPositions.push_back(p3);
            oldPositions.push_back(p4);
            oldPositions.push_back(p5);
            oldPositions.push_back(p6);
            oldPositions.push_back(p7);
            oldPositions.push_back(p8);

            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(MapFormat::Standard);

            BrushBuilder builder(&world, worldBounds);
            Brush brush = builder.createBrush(oldPositions, "texture");

            const std::vector<vm::vec3d> result = brush.moveVertices(worldBounds, std::vector<vm::vec3d>(1, p8), p9 - p8);
            ASSERT_EQ(1u, result.size());
            ASSERT_VEC_EQ(p9, result[0]);

            ASSERT_EQ(7u, brush.vertexCount());
            ASSERT_EQ(12u, brush.edgeCount());
            ASSERT_EQ(7u, brush.faceCount());

            ASSERT_TRUE(brush.hasVertex(p1));
            ASSERT_TRUE(brush.hasVertex(p2));
            ASSERT_TRUE(brush.hasVertex(p3));
            ASSERT_TRUE(brush.hasVertex(p4));
            ASSERT_TRUE(brush.hasVertex(p5));
            ASSERT_TRUE(brush.hasVertex(p7));
            ASSERT_TRUE(brush.hasVertex(p9));

            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p2)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p3)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p5)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p2, p4)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p2, p9)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p3, p4)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p3, p7)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p4, p7)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p4, p9)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p5, p7)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p5, p9)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p7, p9)));

            ASSERT_TRUE(brush.hasFace(p1, p2, p4, p3));
            ASSERT_TRUE(brush.hasFace(p1, p3, p7, p5));
            ASSERT_TRUE(brush.hasFace(p1, p5, p9, p2));
            ASSERT_TRUE(brush.hasFace(p2, p9, p4));
            ASSERT_TRUE(brush.hasFace(p3, p4, p7));
            ASSERT_TRUE(brush.hasFace(p4, p9, p7));
            ASSERT_TRUE(brush.hasFace(p5, p7, p9));
        }

        TEST_CASE("BrushTest.moveVertexAndMergeColinearEdgesWithDeletingVertex", "[BrushTest]") {
            const vm::vec3d p1(-64.0, -64.0, -64.0);
            const vm::vec3d p2(-64.0, -64.0, +64.0);
            const vm::vec3d p3(-64.0, +64.0, -64.0);
            const vm::vec3d p4(-64.0, +64.0, +64.0);
            const vm::vec3d p5(+64.0, -64.0, -64.0);
            const vm::vec3d p6(+64.0, -64.0, +64.0);
            const vm::vec3d p7(+64.0, +64.0, -64.0);
            const vm::vec3d p8(+64.0, +64.0, +64.0);
            const vm::vec3d p9(+80.0, 0.0, +64.0);
            const vm::vec3d p10(+64.0, 0.0, +64.0);

            std::vector<vm::vec3d> oldPositions;
            oldPositions.push_back(p1);
            oldPositions.push_back(p2);
            oldPositions.push_back(p3);
            oldPositions.push_back(p4);
            oldPositions.push_back(p5);
            oldPositions.push_back(p6);
            oldPositions.push_back(p7);
            oldPositions.push_back(p8);
            oldPositions.push_back(p9);

            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(MapFormat::Standard);

            BrushBuilder builder(&world, worldBounds);
            Brush brush = builder.createBrush(oldPositions, "texture");

            const std::vector<vm::vec3d> result = brush.moveVertices(worldBounds, std::vector<vm::vec3d>(1, p9), p10 - p9);
            ASSERT_EQ(0u, result.size());

            ASSERT_EQ(8u, brush.vertexCount());
            ASSERT_EQ(12u, brush.edgeCount());
            ASSERT_EQ(6u, brush.faceCount());

            ASSERT_TRUE(brush.hasVertex(p1));
            ASSERT_TRUE(brush.hasVertex(p2));
            ASSERT_TRUE(brush.hasVertex(p3));
            ASSERT_TRUE(brush.hasVertex(p4));
            ASSERT_TRUE(brush.hasVertex(p5));
            ASSERT_TRUE(brush.hasVertex(p6));
            ASSERT_TRUE(brush.hasVertex(p7));
            ASSERT_TRUE(brush.hasVertex(p8));

            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p2)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p3)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p5)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p2, p4)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p2, p6)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p3, p4)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p3, p7)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p4, p8)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p5, p6)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p5, p7)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p6, p8)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p7, p8)));

            ASSERT_TRUE(brush.hasFace(p1, p2, p4, p3));
            ASSERT_TRUE(brush.hasFace(p1, p3, p7, p5));
            ASSERT_TRUE(brush.hasFace(p1, p5, p6, p2));
            ASSERT_TRUE(brush.hasFace(p2, p6, p8, p4));
            ASSERT_TRUE(brush.hasFace(p3, p4, p8, p7));
            ASSERT_TRUE(brush.hasFace(p5, p7, p8, p6));
        }
        
        TEST_CASE("BrushTest.moveVerticesPastWorldBounds", "[BrushTest]") {
            const vm::bbox3 worldBounds(8192.0);
            WorldNode world(MapFormat::Standard);
            const BrushBuilder builder(&world, worldBounds);

            Model::Brush brush = builder.createCube(128.0, "texture");

            std::vector<vm::vec3> allVertexPositions;
            for (const auto* vertex : brush.vertices()) {
                allVertexPositions.push_back(vertex->position());
            }

            EXPECT_TRUE(brush.canMoveVertices(worldBounds, allVertexPositions, vm::vec3(16, 0, 0)));
            EXPECT_FALSE(brush.canMoveVertices(worldBounds, allVertexPositions, vm::vec3(8192, 0, 0)));
        }
        
        static void assertCanMoveVertices(const Brush& brush, const std::vector<vm::vec3> vertexPositions, const vm::vec3 delta) {
            const vm::bbox3 worldBounds(4096.0);

            ASSERT_TRUE(brush.canMoveVertices(worldBounds, vertexPositions, delta));

            Brush copy = brush;

            auto movedVertexPositions = copy.moveVertices(worldBounds, vertexPositions, delta);
            kdl::vec_sort_and_remove_duplicates(movedVertexPositions);

            auto expectedVertexPositions = vertexPositions + delta;
            kdl::vec_sort_and_remove_duplicates(expectedVertexPositions);

            ASSERT_EQ(expectedVertexPositions, movedVertexPositions);
        }

        // "Move point" tests

        static void assertMovingVerticesDeletes(const Brush& brush, const std::vector<vm::vec3> vertexPositions, const vm::vec3 delta) {
            const vm::bbox3 worldBounds(4096.0);

            ASSERT_TRUE(brush.canMoveVertices(worldBounds, vertexPositions, delta));

            Brush copy = brush;
            const std::vector<vm::vec3> movedVertexPositions = copy.moveVertices(worldBounds, vertexPositions, delta);

            ASSERT_EQ(std::vector<vm::vec3>(), movedVertexPositions);
        }

        static void assertCanNotMoveVertices(const Brush& brush, const std::vector<vm::vec3> vertexPositions, const vm::vec3 delta) {
            const vm::bbox3 worldBounds(4096.0);
            ASSERT_FALSE(brush.canMoveVertices(worldBounds, vertexPositions, delta));
        }

        static void assertCanMoveVertex(const Brush& brush, const vm::vec3 vertexPosition, const vm::vec3 delta) {
            assertCanMoveVertices(brush, std::vector<vm::vec3>{vertexPosition}, delta);
        }

        static void assertMovingVertexDeletes(const Brush& brush, const vm::vec3 vertexPosition, const vm::vec3 delta) {
            assertMovingVerticesDeletes(brush, std::vector<vm::vec3>{vertexPosition}, delta);
        }

        static void assertCanNotMoveVertex(const Brush& brush, const vm::vec3 vertexPosition, const vm::vec3 delta) {
            assertCanNotMoveVertices(brush, std::vector<vm::vec3>{vertexPosition}, delta);
        }

        // NOTE: Different than movePolygonRemainingPoint, because in this case we allow
        // point moves that flip the normal of the remaining polygon
        TEST_CASE("BrushTest.movePointRemainingPolygon", "[BrushTest]") {
            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(MapFormat::Standard);

            const vm::vec3 peakPosition(0.0, 0.0, +64.0);
            const std::vector<vm::vec3> baseQuadVertexPositions{
                    vm::vec3(-64.0, -64.0, -64.0), // base quad
                    vm::vec3(-64.0, +64.0, -64.0),
                    vm::vec3(+64.0, +64.0, -64.0),
                    vm::vec3(+64.0, -64.0, -64.0)
            };
            const std::vector<vm::vec3> vertexPositions = kdl::vec_concat(std::vector<vm::vec3>{ peakPosition },
                baseQuadVertexPositions);

            BrushBuilder builder(&world, worldBounds);
            Brush brush = builder.createBrush(vertexPositions, Model::BrushFaceAttributes::NoTextureName);

            assertCanMoveVertex(brush, peakPosition, vm::vec3(0.0, 0.0, -127.0));
            assertCanNotMoveVertex(brush, peakPosition, vm::vec3(0.0, 0.0, -128.0)); // Onto the base quad plane
            assertCanMoveVertex(brush, peakPosition, vm::vec3(0.0, 0.0, -129.0)); // Through the other side of the base quad

            // More detailed testing of the last assertion
            {
                std::vector<vm::vec3> temp(baseQuadVertexPositions);
                std::reverse(temp.begin(), temp.end());
                const std::vector<vm::vec3> flippedBaseQuadVertexPositions(temp);

                const vm::vec3 delta(0.0, 0.0, -129.0);
                Brush copy = brush;

                ASSERT_EQ(5u, copy.faceCount());
                ASSERT_TRUE(copy.findFace(vm::polygon3(baseQuadVertexPositions)));
                ASSERT_FALSE(copy.findFace(vm::polygon3(flippedBaseQuadVertexPositions)));
                ASSERT_NE(nullptr, copy.findFace(vm::vec3::neg_z()));
                ASSERT_EQ(nullptr, copy.findFace(vm::vec3::pos_z()));

                ASSERT_TRUE(copy.canMoveVertices(worldBounds, std::vector<vm::vec3>{peakPosition}, delta));
                ASSERT_EQ(std::vector<vm::vec3>{peakPosition + delta}, copy.moveVertices(worldBounds, std::vector<vm::vec3>{peakPosition}, delta));

                ASSERT_EQ(5u, copy.faceCount());
                ASSERT_FALSE(copy.findFace(vm::polygon3(baseQuadVertexPositions)));
                ASSERT_TRUE(copy.findFace(vm::polygon3(flippedBaseQuadVertexPositions)));
                ASSERT_EQ(nullptr, copy.findFace(vm::vec3::neg_z()));
                ASSERT_NE(nullptr, copy.findFace(vm::vec3::pos_z()));
            }

            assertCanMoveVertex(brush, peakPosition, vm::vec3(256.0, 0.0, -127.0));
            assertCanNotMoveVertex(brush, peakPosition, vm::vec3(256.0, 0.0, -128.0)); // Onto the base quad plane
            assertCanMoveVertex(brush, peakPosition, vm::vec3(256.0, 0.0, -129.0)); // Flips the normal of the base quad, without moving through it
        }

        TEST_CASE("BrushTest.movePointRemainingPolyhedron", "[BrushTest]") {
            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(MapFormat::Standard);

            const vm::vec3 peakPosition(0.0, 0.0, 128.0);
            const std::vector<vm::vec3> vertexPositions {
                    vm::vec3(-64.0, -64.0, 0.0), // base quad
                    vm::vec3(-64.0, +64.0, 0.0),
                    vm::vec3(+64.0, +64.0, 0.0),
                    vm::vec3(+64.0, -64.0, 0.0),
                    vm::vec3(-64.0, -64.0, 64.0), // upper quad
                    vm::vec3(-64.0, +64.0, 64.0),
                    vm::vec3(+64.0, +64.0, 64.0),
                    vm::vec3(+64.0, -64.0, 64.0),
                    peakPosition
            };

            BrushBuilder builder(&world, worldBounds);
            Brush brush = builder.createBrush(vertexPositions, Model::BrushFaceAttributes::NoTextureName);

            assertMovingVertexDeletes(brush, peakPosition, vm::vec3(0.0, 0.0, -65.0)); // Move inside the remaining cuboid
            assertCanMoveVertex(brush, peakPosition, vm::vec3(0.0, 0.0, -63.0)); // Slightly above the top of the cuboid is OK
            assertCanNotMoveVertex(brush, peakPosition, vm::vec3(0.0, 0.0, -129.0)); // Through and out the other side is disallowed
        }
        
        // add vertex tests
        
        // TODO: add tests for Brush::addVertex
        
        // remove vertex tests
        
        TEST_CASE("BrushTest.removeSingleVertex", "[BrushTest]") {
            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(MapFormat::Standard);

            BrushBuilder builder(&world, worldBounds);
            Brush brush = builder.createCube(64.0, "asdf");


            brush.removeVertices(worldBounds, std::vector<vm::vec3>(1, vm::vec3(+32.0, +32.0, +32.0)));

            ASSERT_EQ(7u, brush.vertexCount());
            ASSERT_TRUE (brush.hasVertex(vm::vec3(-32.0, -32.0, -32.0)));
            ASSERT_TRUE (brush.hasVertex(vm::vec3(-32.0, -32.0, +32.0)));
            ASSERT_TRUE (brush.hasVertex(vm::vec3(-32.0, +32.0, -32.0)));
            ASSERT_TRUE (brush.hasVertex(vm::vec3(-32.0, +32.0, +32.0)));
            ASSERT_TRUE (brush.hasVertex(vm::vec3(+32.0, -32.0, -32.0)));
            ASSERT_TRUE (brush.hasVertex(vm::vec3(+32.0, -32.0, +32.0)));
            ASSERT_TRUE (brush.hasVertex(vm::vec3(+32.0, +32.0, -32.0)));
            ASSERT_FALSE(brush.hasVertex(vm::vec3(+32.0, +32.0, +32.0)));


            brush.removeVertices(worldBounds, std::vector<vm::vec3>(1, vm::vec3(+32.0, +32.0, -32.0)));

            ASSERT_EQ(6u, brush.vertexCount());
            ASSERT_TRUE (brush.hasVertex(vm::vec3(-32.0, -32.0, -32.0)));
            ASSERT_TRUE (brush.hasVertex(vm::vec3(-32.0, -32.0, +32.0)));
            ASSERT_TRUE (brush.hasVertex(vm::vec3(-32.0, +32.0, -32.0)));
            ASSERT_TRUE (brush.hasVertex(vm::vec3(-32.0, +32.0, +32.0)));
            ASSERT_TRUE (brush.hasVertex(vm::vec3(+32.0, -32.0, -32.0)));
            ASSERT_TRUE (brush.hasVertex(vm::vec3(+32.0, -32.0, +32.0)));
            ASSERT_FALSE(brush.hasVertex(vm::vec3(+32.0, +32.0, -32.0)));
            ASSERT_FALSE(brush.hasVertex(vm::vec3(+32.0, +32.0, +32.0)));


            brush.removeVertices(worldBounds, std::vector<vm::vec3>(1, vm::vec3(+32.0, -32.0, +32.0)));

            ASSERT_EQ(5u, brush.vertexCount());
            ASSERT_TRUE (brush.hasVertex(vm::vec3(-32.0, -32.0, -32.0)));
            ASSERT_TRUE (brush.hasVertex(vm::vec3(-32.0, -32.0, +32.0)));
            ASSERT_TRUE (brush.hasVertex(vm::vec3(-32.0, +32.0, -32.0)));
            ASSERT_TRUE (brush.hasVertex(vm::vec3(-32.0, +32.0, +32.0)));
            ASSERT_TRUE (brush.hasVertex(vm::vec3(+32.0, -32.0, -32.0)));
            ASSERT_FALSE(brush.hasVertex(vm::vec3(+32.0, -32.0, +32.0)));
            ASSERT_FALSE(brush.hasVertex(vm::vec3(+32.0, +32.0, -32.0)));
            ASSERT_FALSE(brush.hasVertex(vm::vec3(+32.0, +32.0, +32.0)));


            brush.removeVertices(worldBounds, std::vector<vm::vec3>(1, vm::vec3(-32.0, -32.0, -32.0)));

            ASSERT_EQ(4u, brush.vertexCount());
            ASSERT_FALSE(brush.hasVertex(vm::vec3(-32.0, -32.0, -32.0)));
            ASSERT_TRUE (brush.hasVertex(vm::vec3(-32.0, -32.0, +32.0)));
            ASSERT_TRUE (brush.hasVertex(vm::vec3(-32.0, +32.0, -32.0)));
            ASSERT_TRUE (brush.hasVertex(vm::vec3(-32.0, +32.0, +32.0)));
            ASSERT_TRUE (brush.hasVertex(vm::vec3(+32.0, -32.0, -32.0)));
            ASSERT_FALSE(brush.hasVertex(vm::vec3(+32.0, -32.0, +32.0)));
            ASSERT_FALSE(brush.hasVertex(vm::vec3(+32.0, +32.0, -32.0)));
            ASSERT_FALSE(brush.hasVertex(vm::vec3(+32.0, +32.0, +32.0)));


            ASSERT_FALSE(brush.canRemoveVertices(worldBounds, std::vector<vm::vec3>(1, vm::vec3(-32.0, -32.0, +32.0))));
            ASSERT_FALSE(brush.canRemoveVertices(worldBounds, std::vector<vm::vec3>(1, vm::vec3(-32.0, +32.0, -32.0))));
            ASSERT_FALSE(brush.canRemoveVertices(worldBounds, std::vector<vm::vec3>(1, vm::vec3(-32.0, +32.0, +32.0))));
            ASSERT_FALSE(brush.canRemoveVertices(worldBounds, std::vector<vm::vec3>(1, vm::vec3(+32.0, -32.0, -32.0))));
        }


        TEST_CASE("BrushTest.removeMultipleVertices", "[BrushTest]") {
            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(MapFormat::Standard);
            BrushBuilder builder(&world, worldBounds);

            std::vector<vm::vec3> vertices;
            vertices.push_back(vm::vec3(-32.0, -32.0, -32.0));
            vertices.push_back(vm::vec3(-32.0, -32.0, +32.0));
            vertices.push_back(vm::vec3(-32.0, +32.0, -32.0));
            vertices.push_back(vm::vec3(-32.0, +32.0, +32.0));
            vertices.push_back(vm::vec3(+32.0, -32.0, -32.0));
            vertices.push_back(vm::vec3(+32.0, -32.0, +32.0));
            vertices.push_back(vm::vec3(+32.0, +32.0, -32.0));
            vertices.push_back(vm::vec3(+32.0, +32.0, +32.0));

            for (size_t i = 0; i < 6; ++i) {
                for (size_t j = i + 1; j < 7; ++j) {
                    for (size_t k = j + 1; k < 8; ++k) {
                        std::vector<vm::vec3> toRemove;
                        toRemove.push_back(vertices[i]);
                        toRemove.push_back(vertices[j]);
                        toRemove.push_back(vertices[k]);

                        Brush brush = builder.createBrush(vertices, "asdf");
                        ASSERT_TRUE(brush.canRemoveVertices(worldBounds, toRemove));
                        brush.removeVertices(worldBounds, toRemove);

                        for (size_t l = 0; l < 8; ++l) {
                            if (l != i && l != j && l != k) {
                                ASSERT_TRUE(brush.hasVertex(vertices[l]));
                            }
                        }
                    }
                }
            }
        }

        // snap vertices tests
        
        static void assertCannotSnapTo(const std::string& data, const FloatType gridSize) {
            const vm::bbox3 worldBounds(8192.0);
            WorldNode world(MapFormat::Standard);

            IO::TestParserStatus status;
            IO::NodeReader reader(data, world);

            const std::vector<Node*> nodes = reader.read(worldBounds, status);
            EXPECT_EQ(1u, nodes.size());

            Brush brush = static_cast<BrushNode*>(nodes.front())->brush();
            ASSERT_FALSE(brush.canSnapVertices(worldBounds, gridSize));
            
            kdl::col_delete_all(nodes);
        }

        static void assertCannotSnap(const std::string& data) {
            assertCannotSnapTo(data, 1.0);
        }

        static void assertSnapTo(const std::string& data, const FloatType gridSize) {
            const vm::bbox3 worldBounds(8192.0);
            WorldNode world(MapFormat::Standard);

            IO::TestParserStatus status;
            IO::NodeReader reader(data, world);

            const std::vector<Node*> nodes = reader.read(worldBounds, status);
            EXPECT_EQ(1u, nodes.size());

            Brush brush = static_cast<BrushNode*>(nodes.front())->brush();
            ASSERT_TRUE(brush.canSnapVertices(worldBounds, gridSize));

            brush.snapVertices(worldBounds, gridSize);
            ASSERT_TRUE(brush.fullySpecified());

            // Ensure they were actually snapped
            {
                for (const Model::BrushVertex* vertex : brush.vertices()) {
                    const vm::vec3& pos = vertex->position();
                    ASSERT_TRUE(vm::is_integral(pos, 0.001));
                }
            }
            
            kdl::col_delete_all(nodes);
        }

        static void assertSnapToInteger(const std::string& data) {
            assertSnapTo(data, 1.0);
        }

        // TODO: add tests for Brush::snapVertices (there are some issue tests below)
                
        // "Move edge" tests

        static void assertCanMoveEdges(const Brush& brush, const std::vector<vm::segment3> edges, const vm::vec3 delta) {
            const vm::bbox3 worldBounds(4096.0);

            std::vector<vm::segment3> expectedMovedEdges;
            for (const vm::segment3& edge : edges) {
                expectedMovedEdges.push_back(vm::segment3(edge.start() + delta, edge.end() + delta));
            }

            ASSERT_TRUE(brush.canMoveEdges(worldBounds, edges, delta));

            Brush copy = brush;
            const std::vector<vm::segment3> movedEdges = copy.moveEdges(worldBounds, edges, delta);

            ASSERT_EQ(expectedMovedEdges, movedEdges);
        }

        static void assertCanNotMoveEdges(const Brush& brush, const std::vector<vm::segment3> edges, const vm::vec3 delta) {
            const vm::bbox3 worldBounds(4096.0);
            ASSERT_FALSE(brush.canMoveEdges(worldBounds, edges, delta));
        }

        TEST_CASE("BrushTest.moveEdgeRemainingPolyhedron", "[BrushTest]") {
            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(MapFormat::Standard);

            // Taller than the cube, starts to the left of the +-64 unit cube
            const vm::segment3 edge(vm::vec3(-128, 0, -128), vm::vec3(-128, 0, +128));

            BrushBuilder builder(&world, worldBounds);
            Brush brush = builder.createCube(128, Model::BrushFaceAttributes::NoTextureName);
            ASSERT_NE(nullptr, brush.addVertex(worldBounds, edge.start()));
            ASSERT_NE(nullptr, brush.addVertex(worldBounds, edge.end()));

            ASSERT_EQ(10u, brush.vertexCount());

            assertCanMoveEdges(brush, std::vector<vm::segment3>{edge}, vm::vec3(+63, 0, 0));
            assertCanNotMoveEdges(brush, std::vector<vm::segment3>{edge}, vm::vec3(+64, 0, 0)); // On the side of the cube
            assertCanNotMoveEdges(brush, std::vector<vm::segment3>{edge}, vm::vec3(+128, 0, 0)); // Center of the cube

            assertCanMoveVertices(brush, asVertexList(std::vector<vm::segment3>{edge}), vm::vec3(+63, 0, 0));
            assertCanMoveVertices(brush, asVertexList(std::vector<vm::segment3>{edge}), vm::vec3(+64, 0, 0));
            assertCanMoveVertices(brush, asVertexList(std::vector<vm::segment3>{edge}), vm::vec3(+128, 0, 0));
        }

        // Same as above, but moving 2 edges
        TEST_CASE("BrushTest.moveEdgesRemainingPolyhedron", "[BrushTest]") {
            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(MapFormat::Standard);

            // Taller than the cube, starts to the left of the +-64 unit cube
            const vm::segment3 edge1(vm::vec3(-128, -32, -128), vm::vec3(-128, -32, +128));
            const vm::segment3 edge2(vm::vec3(-128, +32, -128), vm::vec3(-128, +32, +128));
            const std::vector<vm::segment3> movingEdges{edge1, edge2};

            BrushBuilder builder(&world, worldBounds);
            Brush brush = builder.createCube(128, Model::BrushFaceAttributes::NoTextureName);
            ASSERT_NE(nullptr, brush.addVertex(worldBounds, edge1.start()));
            ASSERT_NE(nullptr, brush.addVertex(worldBounds, edge1.end()));
            ASSERT_NE(nullptr, brush.addVertex(worldBounds, edge2.start()));
            ASSERT_NE(nullptr, brush.addVertex(worldBounds, edge2.end()));

            ASSERT_EQ(12u, brush.vertexCount());

            assertCanMoveEdges(brush, movingEdges, vm::vec3(+63, 0, 0));
            assertCanNotMoveEdges(brush, movingEdges, vm::vec3(+64, 0, 0)); // On the side of the cube
            assertCanNotMoveEdges(brush, movingEdges, vm::vec3(+128, 0, 0)); // Center of the cube

            assertCanMoveVertices(brush, asVertexList(movingEdges), vm::vec3(+63, 0, 0));
            assertCanMoveVertices(brush, asVertexList(movingEdges), vm::vec3(+64, 0, 0));
            assertCanMoveVertices(brush, asVertexList(movingEdges), vm::vec3(+128, 0, 0));
        }

        // "Move polygon" tests
        
        static void assertCanMoveFaces(const Brush& brush, const std::vector<vm::polygon3> movingFaces, const vm::vec3 delta) {
            const vm::bbox3 worldBounds(4096.0);

            std::vector<vm::polygon3> expectedMovedFaces;
            for (const vm::polygon3& polygon : movingFaces) {
                expectedMovedFaces.push_back(vm::polygon3(polygon.vertices() + delta));
            }

            ASSERT_TRUE(brush.canMoveFaces(worldBounds, movingFaces, delta));

            Brush copy = brush;
            const std::vector<vm::polygon3> movedFaces = copy.moveFaces(worldBounds, movingFaces, delta);
            ASSERT_EQ(expectedMovedFaces, movedFaces);
        }

        static void assertCanNotMoveFaces(const Brush& brush, const std::vector<vm::polygon3> movingFaces, const vm::vec3 delta) {
            const vm::bbox3 worldBounds(4096.0);
            ASSERT_FALSE(brush.canMoveFaces(worldBounds, movingFaces, delta));
        }

        static void assertCanMoveFace(const Brush& brush, const BrushFace* topFace, const vm::vec3 delta) {
            assertCanMoveFaces(brush, std::vector<vm::polygon3>{topFace->polygon()}, delta);
        }

        static void assertCanNotMoveFace(const Brush& brush, const BrushFace* topFace, const vm::vec3 delta) {
            const vm::bbox3 worldBounds(4096.0);

            EXPECT_NE(nullptr, topFace);

            ASSERT_FALSE(brush.canMoveFaces(worldBounds, std::vector<vm::polygon3>{topFace->polygon()}, delta));
        }

        static void assertCanMoveTopFace(const Brush& brush, const vm::vec3 delta) {
            assertCanMoveFace(brush, brush.findFace(vm::vec3::pos_z()), delta);
        }

        static void assertCanNotMoveTopFace(const Brush& brush, const vm::vec3 delta) {
            assertCanNotMoveFace(brush, brush.findFace(vm::vec3::pos_z()), delta);
        }

        static void assertCanNotMoveTopFaceBeyond127UnitsDown(const Brush& brush) {
            assertCanMoveTopFace(brush, vm::vec3(0, 0, -127));
            assertCanNotMoveTopFace(brush, vm::vec3(0, 0, -128));
            assertCanNotMoveTopFace(brush, vm::vec3(0, 0, -129));

            assertCanMoveTopFace(brush, vm::vec3(256, 0, -127));
            assertCanNotMoveTopFace(brush, vm::vec3(256, 0, -128));
            assertCanNotMoveTopFace(brush, vm::vec3(256, 0, -129));
        }
        
        TEST_CASE("BrushTest.movePolygonRemainingPoint", "[BrushTest]") {
            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(MapFormat::Standard);

            const std::vector<vm::vec3> vertexPositions{
                    vm::vec3(-64.0, -64.0, +64.0), // top quad
                    vm::vec3(-64.0, +64.0, +64.0),
                    vm::vec3(+64.0, -64.0, +64.0),
                    vm::vec3(+64.0, +64.0, +64.0),

                    vm::vec3(0.0, 0.0, -64.0), // bottom point
            };

            BrushBuilder builder(&world, worldBounds);
            Brush brush = builder.createBrush(vertexPositions, Model::BrushFaceAttributes::NoTextureName);

            assertCanNotMoveTopFaceBeyond127UnitsDown(brush);
        }

        TEST_CASE("BrushTest.movePolygonRemainingEdge", "[BrushTest]") {
            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(MapFormat::Standard);

            const std::vector<vm::vec3> vertexPositions{
                    vm::vec3(-64.0, -64.0, +64.0), // top quad
                    vm::vec3(-64.0, +64.0, +64.0),
                    vm::vec3(+64.0, -64.0, +64.0),
                    vm::vec3(+64.0, +64.0, +64.0),

                    vm::vec3(-64.0, 0.0, -64.0), // bottom edge, on the z=-64 plane
                    vm::vec3(+64.0, 0.0, -64.0)
            };

            BrushBuilder builder(&world, worldBounds);
            Brush brush = builder.createBrush(vertexPositions, Model::BrushFaceAttributes::NoTextureName);

            assertCanNotMoveTopFaceBeyond127UnitsDown(brush);
        }

        TEST_CASE("BrushTest.movePolygonRemainingPolygon", "[BrushTest]") {
            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(MapFormat::Standard);

            BrushBuilder builder(&world, worldBounds);
            Brush brush = builder.createCube(128.0, Model::BrushFaceAttributes::NoTextureName);

            assertCanNotMoveTopFaceBeyond127UnitsDown(brush);
        }

        TEST_CASE("BrushTest.movePolygonRemainingPolygon2", "[BrushTest]") {
            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(MapFormat::Standard);

            // Same brush as movePolygonRemainingPolygon, but this particular order of vertices triggers a failure in Brush::doCanMoveVertices
            // where the polygon inserted into the "remaining" BrushGeometry gets the wrong normal.
            const std::vector<vm::vec3> vertexPositions{
                    vm::vec3(+64.0, +64.0, +64.0),
                    vm::vec3(+64.0, -64.0, +64.0),
                    vm::vec3(+64.0, -64.0, -64.0),
                    vm::vec3(+64.0, +64.0, -64.0),
                    vm::vec3(-64.0, -64.0, +64.0),
                    vm::vec3(-64.0, -64.0, -64.0),
                    vm::vec3(-64.0, +64.0, -64.0),
                    vm::vec3(-64.0, +64.0, +64.0)};

            BrushBuilder builder(&world, worldBounds);
            Brush brush = builder.createBrush(vertexPositions, Model::BrushFaceAttributes::NoTextureName);
            ASSERT_EQ(vm::bbox3(vm::vec3(-64, -64, -64), vm::vec3(64, 64, 64)), brush.bounds());

            assertCanNotMoveTopFaceBeyond127UnitsDown(brush);
        }

        TEST_CASE("BrushTest.movePolygonRemainingPolygon_DisallowVertexCombining", "[BrushTest]") {
            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(MapFormat::Standard);

            //       z = +192  //
            // |\              //
            // | \             //
            // |  \  z = +64   //
            // |   |           //
            // |___| z = -64   //
            //                 //

            const std::vector<vm::vec3> vertexPositions{
                    vm::vec3(-64.0, -64.0, +192.0), // top quad, slanted
                    vm::vec3(-64.0, +64.0, +192.0),
                    vm::vec3(+64.0, -64.0, +64.0),
                    vm::vec3(+64.0, +64.0, +64.0),

                    vm::vec3(-64.0, -64.0, -64.0), // bottom quad
                    vm::vec3(-64.0, +64.0, -64.0),
                    vm::vec3(+64.0, -64.0, -64.0),
                    vm::vec3(+64.0, +64.0, -64.0),
            };

            const vm::vec3 topFaceNormal(sqrt(2.0) / 2.0, 0.0, sqrt(2.0) / 2.0);

            BrushBuilder builder(&world, worldBounds);
            Brush brush = builder.createBrush(vertexPositions, Model::BrushFaceAttributes::NoTextureName);

            BrushFace* topFace = brush.findFace(topFaceNormal);
            EXPECT_NE(nullptr, topFace);

            assertCanMoveFace(brush, topFace, vm::vec3(0, 0, -127));
            assertCanMoveFace(brush, topFace, vm::vec3(0, 0, -128)); // Merge 2 verts of the moving polygon with 2 in the remaining polygon, should be allowed
            assertCanNotMoveFace(brush, topFace, vm::vec3(0, 0, -129));
        }

        TEST_CASE("BrushTest.movePolygonRemainingPolyhedron", "[BrushTest]") {
            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(MapFormat::Standard);

            //   _   z = +64   //
            //  / \            //
            // /   \           //
            // |   | z = -64   //
            // |   |           //
            // |___| z = -192  //
            //                 //

            const std::vector<vm::vec3> smallerTopPolygon{
                    vm::vec3(-32.0, -32.0, +64.0), // smaller top polygon
                    vm::vec3(-32.0, +32.0, +64.0),
                    vm::vec3(+32.0, -32.0, +64.0),
                    vm::vec3(+32.0, +32.0, +64.0)
            };
            const std::vector<vm::vec3> cubeTopFace{
                    vm::vec3(-64.0, -64.0, -64.0), // top face of cube
                    vm::vec3(-64.0, +64.0, -64.0),
                    vm::vec3(+64.0, -64.0, -64.0),
                    vm::vec3(+64.0, +64.0, -64.0),
            };
            const std::vector<vm::vec3> cubeBottomFace{
                    vm::vec3(-64.0, -64.0, -192.0), // bottom face of cube
                    vm::vec3(-64.0, +64.0, -192.0),
                    vm::vec3(+64.0, -64.0, -192.0),
                    vm::vec3(+64.0, +64.0, -192.0),
            };

            const std::vector<vm::vec3> vertexPositions = kdl::vec_concat(smallerTopPolygon, cubeTopFace,
                cubeBottomFace);

            BrushBuilder builder(&world, worldBounds);
            Brush brush = builder.createBrush(vertexPositions, Model::BrushFaceAttributes::NoTextureName);

            // Try to move the top face down along the Z axis
            assertCanNotMoveTopFaceBeyond127UnitsDown(brush);
            assertCanNotMoveTopFace(brush, vm::vec3(0.0, 0.0, -257.0)); // Move top through the polyhedron and out the bottom

            // Move the smaller top polygon as 4 separate vertices
            assertCanMoveVertices(brush, smallerTopPolygon, vm::vec3(0, 0, -127));
            assertMovingVerticesDeletes(brush, smallerTopPolygon, vm::vec3(0, 0, -128));
            assertMovingVerticesDeletes(brush, smallerTopPolygon, vm::vec3(0, 0, -129));
            assertCanNotMoveVertices(brush, smallerTopPolygon, vm::vec3(0, 0, -257)); // Move through the polyhedron and out the bottom

            // Move top face along the X axis
            assertCanMoveTopFace(brush, vm::vec3(32.0, 0.0, 0.0));
            assertCanMoveTopFace(brush, vm::vec3(256, 0.0, 0.0));
            assertCanMoveTopFace(brush, vm::vec3(-32.0, -32.0, 0.0)); // Causes face merging and a vert to be deleted at z=-64
        }

        TEST_CASE("BrushTest.moveTwoFaces", "[BrushTest]") {
            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(MapFormat::Standard);

            //               //
            // |\    z = 64  //
            // | \           //
            // |  \          //
            //A|   \ z = 0   //
            // |   /         //
            // |__/C         //
            //  B    z = -64 //
            //               //


            const std::vector<vm::vec3> leftPolygon{ // A
                    vm::vec3(-32.0, -32.0, +64.0),
                    vm::vec3(-32.0, +32.0, +64.0),
                    vm::vec3(-32.0, +32.0, -64.0),
                    vm::vec3(-32.0, -32.0, -64.0),
            };
            const std::vector<vm::vec3> bottomPolygon{ // B
                    vm::vec3(-32.0, -32.0, -64.0),
                    vm::vec3(-32.0, +32.0, -64.0),
                    vm::vec3(+0.0, +32.0, -64.0),
                    vm::vec3(+0.0, -32.0, -64.0),
            };
            const std::vector<vm::vec3> bottomRightPolygon{ // C
                    vm::vec3(+0.0, -32.0, -64.0),
                    vm::vec3(+0.0, +32.0, -64.0),
                    vm::vec3(+32.0, +32.0, +0.0),
                    vm::vec3(+32.0, -32.0, +0.0),
            };

            const std::vector<vm::vec3> vertexPositions = kdl::vec_concat(leftPolygon, bottomPolygon,
                bottomRightPolygon);

            BrushBuilder builder(&world, worldBounds);
            Brush brush = builder.createBrush(vertexPositions, Model::BrushFaceAttributes::NoTextureName);

            EXPECT_TRUE(brush.hasFace(vm::polygon3(leftPolygon)));
            EXPECT_TRUE(brush.hasFace(vm::polygon3(bottomPolygon)));
            EXPECT_TRUE(brush.hasFace(vm::polygon3(bottomRightPolygon)));

            assertCanMoveFaces(brush, std::vector<vm::polygon3>{ vm::polygon3(leftPolygon), vm::polygon3(bottomPolygon) }, vm::vec3(0, 0, 63));
            assertCanNotMoveFaces(brush, std::vector<vm::polygon3>{ vm::polygon3(leftPolygon), vm::polygon3(bottomPolygon) }, vm::vec3(0, 0, 64)); // Merges B and C
        }
        
        // "Move polyhedron" tests

        TEST_CASE("BrushNodeTest.movePolyhedronRemainingEdge", "[BrushNodeTest]") {
            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(MapFormat::Standard);

            // Edge to the left of the cube, shorter, extends down to Z=-256
            const vm::segment3 edge(vm::vec3(-128, 0, -256), vm::vec3(-128, 0, 0));

            BrushBuilder builder(&world, worldBounds);
            Brush brush = builder.createCube(128, Model::BrushFaceAttributes::NoTextureName);
            ASSERT_NE(nullptr, brush.addVertex(worldBounds, edge.start()));
            ASSERT_NE(nullptr, brush.addVertex(worldBounds, edge.end()));

            ASSERT_EQ(10u, brush.vertexCount());

            BrushFace* cubeTop = brush.findFace(vm::vec3::pos_z());
            BrushFace* cubeBottom = brush.findFace(vm::vec3::neg_z());
            BrushFace* cubeRight = brush.findFace(vm::vec3::pos_x());
            BrushFace* cubeLeft = brush.findFace(vm::vec3::neg_x());
            BrushFace* cubeBack = brush.findFace(vm::vec3::pos_y());
            BrushFace* cubeFront = brush.findFace(vm::vec3::neg_y());

            EXPECT_NE(nullptr, cubeTop);
            EXPECT_EQ(nullptr, cubeBottom); // no face here, part of the wedge connecting to `edge`
            EXPECT_NE(nullptr, cubeRight);
            EXPECT_EQ(nullptr, cubeLeft); // no face here, part of the wedge connecting to `edge`
            EXPECT_NE(nullptr, cubeFront);
            EXPECT_NE(nullptr, cubeBack);

            const std::vector<vm::polygon3> movingFaces{
                    cubeTop->polygon(),
                    cubeRight->polygon(),
                    cubeFront->polygon(),
                    cubeBack->polygon(),
            };

            assertCanMoveFaces(brush, movingFaces, vm::vec3(32, 0, 0)); // away from `edge`
            assertCanMoveFaces(brush, movingFaces, vm::vec3(-63, 0, 0)); // towards `edge`, not touching
            assertCanMoveFaces(brush, movingFaces, vm::vec3(-64, 0, 0)); // towards `edge`, touching
            assertCanMoveFaces(brush, movingFaces, vm::vec3(-65, 0, 0)); // towards `edge`, covering

            // Move the cube down 64 units, so the top vertex of `edge` is on the same plane as `cubeTop`
            // This will turn `cubeTop` from a quad into a pentagon
            assertCanNotMoveFaces(brush, movingFaces, vm::vec3(0, 0, -64));
            assertCanMoveVertices(brush, asVertexList(movingFaces), vm::vec3(0, 0, -64));

            // Make edge poke through the top face
            assertCanNotMoveFaces(brush, movingFaces, vm::vec3(-192, 0, -128));
            assertCanNotMoveVertices(brush, asVertexList(movingFaces), vm::vec3(-192, 0, -128));
        }
        
        TEST_CASE("BrushTest.moveVertexFailing1", "[BrushTest]") {
            const vm::vec3d p1(-64.0, -64.0, 0.0);
            const vm::vec3d p2(+64.0, -64.0, 0.0);
            const vm::vec3d p3(0.0, +64.0, 0.0);
            const vm::vec3d p4(0.0, 0.0, +32.0);

            std::vector<vm::vec3d> oldPositions;
            oldPositions.push_back(p1);
            oldPositions.push_back(p2);
            oldPositions.push_back(p3);
            oldPositions.push_back(p4);

            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(MapFormat::Standard);

            BrushBuilder builder(&world, worldBounds);
            Brush brush = builder.createBrush(oldPositions, "texture");

            for (size_t i = 0; i < oldPositions.size(); ++i) {
                for (size_t j = 0; j < oldPositions.size(); ++j) {
                    if (i != j) {
                        ASSERT_FALSE(brush.canMoveVertices(worldBounds, std::vector<vm::vec3d>(1, oldPositions[i]), oldPositions[j] - oldPositions[i]));
                    }
                }
            }
        }

        TEST_CASE("BrushTest.moveVertexFail_2158", "[BrushTest]") {
            // see https://github.com/kduske/TrenchBroom/issues/2158
            const std::string data("{\n"
                              "( 320 256 320 ) ( 384 192 320 ) ( 352 224 384 ) sky1 0 96 0 1 1\n"
                              "( 384 128 320 ) ( 320 64 320 ) ( 352 96 384 ) sky1 0 96 0 1 1\n"
                              "( 384 32 320 ) ( 384 32 384 ) ( 384 256 384 ) sky1 0 96 0 1 1\n"
                              "( 192 192 320 ) ( 256 256 320 ) ( 224 224 384 ) sky1 0 96 0 1 1\n"
                              "( 256 64 320 ) ( 192 128 320 ) ( 224 96 384 ) sky1 0 96 0 1 1\n"
                              "( 192 32 384 ) ( 192 32 320 ) ( 192 256 320 ) sky1 0 96 0 1 1\n"
                              "( 384 256 320 ) ( 384 256 384 ) ( 192 256 384 ) sky1 0 96 0 1 1\n"
                              "( 320 64 320 ) ( 256 64 320 ) ( 288 64 384 ) sky1 0 96 0 1 1\n"
                              "( 192 64 352 ) ( 192 240 352 ) ( 368 240 352 ) sky1 0 0 0 1 1\n"
                              "( 384 240 320 ) ( 208 240 320 ) ( 208 64 320 ) sky1 0 0 0 1 1\n"
                              "}\n");

            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(MapFormat::Standard);

            IO::TestParserStatus status;
            IO::NodeReader reader(data, world);

            const std::vector<Node*> nodes = reader.read(worldBounds, status);
            EXPECT_EQ(1u, nodes.size());

            Brush brush = static_cast<BrushNode*>(nodes.front())->brush();
            const vm::vec3 p(192.0, 128.0, 352.0);
            const vm::vec3 d = 4.0 * 16.0 * vm::vec3::neg_y();
            const std::vector<vm::vec3> newPositions = brush.moveVertices(worldBounds, std::vector<vm::vec3>(1, p), d);
            ASSERT_EQ(1u, newPositions.size());
            ASSERT_VEC_EQ(p + d, newPositions.front());
            
            kdl::col_delete_all(nodes);
        }

        TEST_CASE("BrushTest.moveVerticesFail_2158", "[BrushTest]") {
            // see https://github.com/kduske/TrenchBroom/issues/2158
            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(MapFormat::Standard);

            const std::string data = R"(
{
( 404.63242807195160822 -1696.09174007488900315 211.96202895796943722 ) ( 1195.3323608207340385 -1812.61180985669875554 293.31661882168685906 ) ( 415.37140289843625851 -1630.10750076058616287 474.93304004273147712 ) rock4_2 30.92560005187988281 0.960906982421875 5.59741020202636719 0.98696297407150269 0.98029798269271851
( 1164.16895096277721677 -1797.72592376172019613 578.31488545196270934 ) ( 1195.3323608207340385 -1812.61180985669875554 293.31661882168685906 ) ( 1169.17641562068342864 -1800.29610138592852309 568.7974852992444994 ) rock4_2 67.89600372314453125 -61.20909881591796875 13.658599853515625 0.85491102933883667 1.12606000900268555
( 415.37140289843625851 -1630.10750076058616287 474.93304004273147712 ) ( 1195.3323608207340385 -1812.61180985669875554 293.31661882168685906 ) ( 1164.16895096277721677 -1797.72592376172019613 578.31488545196270934 ) rock4_2 -3.77819991111755371 -44.42710113525390625 7.24881982803344727 0.95510202646255493 1.04886996746063232
( 1199.73437537143649934 -1850.52292721460958091 299.11555748386712139 ) ( 1169.18149090383781186 -1800.30190582364161855 568.76530164709924975 ) ( 1195.3323608207340385 -1812.61180985669875554 293.31661882168685906 ) rock4_2 77.66159820556640625 -86.74199676513671875 173.0970001220703125 1.15471994876861572 -1.11249995231628418
( 1195.3323608207340385 -1812.61180985669875554 293.31661882168685906 ) ( 1169.18149090383781186 -1800.30190582364161855 568.76530164709924975 ) ( 1169.17641562068342864 -1800.29610138592852309 568.7974852992444994 ) rock4_2 115.52100372314453125 55.40819931030273438 157.998992919921875 1.19368994235992432 -1.0113600492477417
( 1120.512868445862523 -1855.31927395340585463 574.535634983251839 ) ( 1126.49874461573472217 -1839.25626760914360602 608.06151113412647646 ) ( 1183.69438641028636994 -1904.94288073521306615 311.88345805427366031 ) rock4_2 29.0522003173828125 16.1511993408203125 198.899993896484375 0.90696299076080322 -1.06921005249023438
( 1183.69438641028636994 -1904.94288073521306615 311.88345805427366031 ) ( 1126.49874461573472217 -1839.25626760914360602 608.06151113412647646 ) ( 1163.51855729802718997 -1820.79407602155902168 554.17919393113811566 ) rock4_2 -52.78820037841796875 -84.4026031494140625 200.2100067138671875 0.88777101039886475 -0.97177797555923462
( 1163.51855729802718997 -1820.79407602155902168 554.17919393113811566 ) ( 1126.49874461573472217 -1839.25626760914360602 608.06151113412647646 ) ( 1169.17641562068342864 -1800.29610138592852309 568.7974852992444994 ) rock4_2 72.63649749755859375 102.17099761962890625 80.11309814453125 0.87609797716140747 -1.61881005764007568
( 1169.17641562068342864 -1800.29610138592852309 568.7974852992444994 ) ( 1126.49874461573472217 -1839.25626760914360602 608.06151113412647646 ) ( 1164.16895096277721677 -1797.72592376172019613 578.31488545196270934 ) rock4_2 -0.7561039924621582 32.18519973754882812 75.325897216796875 0.90074300765991211 -1.72079002857208252
( 1183.69438641028636994 -1904.94288073521306615 311.88345805427366031 ) ( 1169.18149090383781186 -1800.30190582364161855 568.76530164709924975 ) ( 1199.73437537143649934 -1850.52292721460958091 299.11555748386712139 ) rock4_2 85.426300048828125 -37.61460113525390625 170.2440032958984375 0.94236099720001221 -1.08232998847961426
( 1169.17641562068342864 -1800.29610138592852309 568.7974852992444994 ) ( 1169.18149090383781186 -1800.30190582364161855 568.76530164709924975 ) ( 1183.69438641028636994 -1904.94288073521306615 311.88345805427366031 ) rock4_2 -15.04969978332519531 -12.76039981842041016 176.2700042724609375 0.93921899795532227 -1.1466900110244751
( 1164.16895096277721677 -1797.72592376172019613 578.31488545196270934 ) ( 1126.49874461573472217 -1839.25626760914360602 608.06151113412647646 ) ( 1164.16844274448340002 -1797.72618014395857244 578.31529060850652968 ) rock4_2 -1.02465999126434326 60.25889968872070312 159.8549957275390625 0.78085201978683472 -1.21036994457244873
( 415.37140289843625851 -1630.10750076058616287 474.93304004273147712 ) ( 409.86763191010521723 -1638.4154691593678308 480.83629920333873997 ) ( 394.84298436650840358 -1643.95107488440089583 473.74271495432344636 ) rock4_2 86.87239837646484375 40.37289810180664062 129.878997802734375 0.66983801126480103 -2.06800007820129395
( 394.84298436650840358 -1643.95107488440089583 473.74271495432344636 ) ( 409.86763191010521723 -1638.4154691593678308 480.83629920333873997 ) ( 417.39145642527222435 -1674.70943252244819632 496.15546600960624346 ) rock4_2 77.13539886474609375 119.01000213623046875 358.319000244140625 1.14928996562957764 1.19559001922607422
( 404.63242807195160822 -1696.09174007488900315 211.96202895796943722 ) ( 415.37140289843625851 -1630.10750076058616287 474.93304004273147712 ) ( 394.84298436650840358 -1643.95107488440089583 473.74271495432344636 ) rock4_2 -19.27930068969726562 17.50340080261230469 148.16400146484375 1.01748001575469971 -0.89703798294067383
( 404.63242807195160822 -1696.09174007488900315 211.96202895796943722 ) ( 383.59438380944988012 -1744.18320926297974438 267.01713311064645495 ) ( 392.51561748944976671 -1758.13841025977330901 221.93166373893632226 ) rock4_2 -43.56299972534179688 -73.20639801025390625 350.87200927734375 0.98191499710083008 1.14552998542785645
( 394.84298436650840358 -1643.95107488440089583 473.74271495432344636 ) ( 383.59438380944988012 -1744.18320926297974438 267.01713311064645495 ) ( 404.63242807195160822 -1696.09174007488900315 211.96202895796943722 ) rock4_2 -57.5941009521484375 20.35930061340332031 349.8599853515625 0.91973602771759033 1.05388998985290527
( 718.09496664767948459 -1851.18753444490516813 378.79962463045302457 ) ( 1120.512868445862523 -1855.31927395340585463 574.535634983251839 ) ( 685.205227597987232 -1880.05386294480922516 267.14020489435648642 ) rock4_2 84.4087982177734375 44.97620010375976562 5.90301990509033203 0.94212800264358521 1.00434005260467529
( 685.205227597987232 -1880.05386294480922516 267.14020489435648642 ) ( 647.29885930542945971 -1801.53486617151679638 462.0987669933149391 ) ( 718.09496664767948459 -1851.18753444490516813 378.79962463045302457 ) rock4_2 -4.20452976226806641 26.958099365234375 7.14522981643676758 0.90771502256393433 1.01380002498626709
( 428.68162139174597769 -1687.29811786616778591 488.88114395300908654 ) ( 1126.49874461573472217 -1839.25626760914360602 608.06151113412647646 ) ( 647.29885930542945971 -1801.53486617151679638 462.0987669933149391 ) rock4_2 -81.561798095703125 -95.4485015869140625 40.62070083618164062 0.5180240273475647 1.46343004703521729
( 647.29885930542945971 -1801.53486617151679638 462.0987669933149391 ) ( 1126.49874461573472217 -1839.25626760914360602 608.06151113412647646 ) ( 1120.512868445862523 -1855.31927395340585463 574.535634983251839 ) rock4_2 52.8777008056640625 -9.35947036743164062 58.6305999755859375 0.61474400758743286 1.24004995822906494
( 417.39145642527222435 -1674.70943252244819632 496.15546600960624346 ) ( 1126.49874461573472217 -1839.25626760914360602 608.06151113412647646 ) ( 428.68162139174597769 -1687.29811786616778591 488.88114395300908654 ) rock4_2 -45.87020111083984375 -44.08499908447265625 41.31510162353515625 0.53462702035903931 1.54106998443603516
( 647.29885930542945971 -1801.53486617151679638 462.0987669933149391 ) ( 1120.512868445862523 -1855.31927395340585463 574.535634983251839 ) ( 718.09496664767948459 -1851.18753444490516813 378.79962463045302457 ) rock4_2 8.81488037109375 37.412200927734375 6.29719018936157227 0.96984899044036865 0.99895197153091431
( 392.51561748944976671 -1758.13841025977330901 221.93166373893632226 ) ( 383.59438380944988012 -1744.18320926297974438 267.01713311064645495 ) ( 685.205227597987232 -1880.05386294480922516 267.14020489435648642 ) rock4_2 5.92700004577636719 4.41837978363037109 8.78011035919189453 0.7744939923286438 1.05709004402160645
( 685.205227597987232 -1880.05386294480922516 267.14020489435648642 ) ( 383.59438380944988012 -1744.18320926297974438 267.01713311064645495 ) ( 647.29885930542945971 -1801.53486617151679638 462.0987669933149391 ) rock4_2 0.02703860029578209 11.37539958953857422 8.51169967651367188 0.77832400798797607 1.01610994338989258
( 647.29885930542945971 -1801.53486617151679638 462.0987669933149391 ) ( 383.59438380944988012 -1744.18320926297974438 267.01713311064645495 ) ( 428.68162139174597769 -1687.29811786616778591 488.88114395300908654 ) rock4_2 75.124298095703125 3.1680600643157959 8.79839038848876953 0.75931602716445923 1.01523995399475098
( 428.68162139174597769 -1687.29811786616778591 488.88114395300908654 ) ( 383.59438380944988012 -1744.18320926297974438 267.01713311064645495 ) ( 417.39145642527222435 -1674.70943252244819632 496.15546600960624346 ) rock4_2 -13.265899658203125 -8.93752956390380859 11.75290012359619141 0.59300100803375244 0.97339397668838501
( 417.39145642527222435 -1674.70943252244819632 496.15546600960624346 ) ( 383.59438380944988012 -1744.18320926297974438 267.01713311064645495 ) ( 394.84298436650840358 -1643.95107488440089583 473.74271495432344636 ) rock4_2 5.71436023712158203 66.92310333251953125 162.699005126953125 0.74939501285552979 -1.05348002910614014
( 409.86763191010521723 -1638.4154691593678308 480.83629920333873997 ) ( 1126.49874461573472217 -1839.25626760914360602 608.06151113412647646 ) ( 417.39145642527222435 -1674.70943252244819632 496.15546600960624346 ) rock4_2 47.94699859619140625 80.93849945068359375 350.2969970703125 0.99699199199676514 0.93575799465179443
( 415.37140289843625851 -1630.10750076058616287 474.93304004273147712 ) ( 1126.49874461573472217 -1839.25626760914360602 608.06151113412647646 ) ( 409.86763191010521723 -1638.4154691593678308 480.83629920333873997 ) rock4_2 -17.06769943237304688 76.29920196533203125 226.9109954833984375 0.86038202047348022 -0.97620397806167603
( 1164.16844274448340002 -1797.72618014395857244 578.31529060850652968 ) ( 1126.49874461573472217 -1839.25626760914360602 608.06151113412647646 ) ( 415.37140289843625851 -1630.10750076058616287 474.93304004273147712 ) rock4_2 17.15080070495605469 78.2032012939453125 226.90899658203125 0.86016601324081421 -0.97621601819992065
( 1164.16895096277721677 -1797.72592376172019613 578.31488545196270934 ) ( 1164.16844274448340002 -1797.72618014395857244 578.31529060850652968 ) ( 415.37140289843625851 -1630.10750076058616287 474.93304004273147712 ) rock4_2 67.65200042724609375 17.70070075988769531 124.0709991455078125 0.93583697080612183 0.99498897790908813
( 685.205227597987232 -1880.05386294480922516 267.14020489435648642 ) ( 1120.512868445862523 -1855.31927395340585463 574.535634983251839 ) ( 1183.69438641028636994 -1904.94288073521306615 311.88345805427366031 ) rock4_2 34.074798583984375 -67.4031982421875 5.12918996810913086 0.89313501119613647 0.99598902463912964
( 685.205227597987232 -1880.05386294480922516 267.14020489435648642 ) ( 1183.69438641028636994 -1904.94288073521306615 311.88345805427366031 ) ( 1199.73437537143649934 -1850.52292721460958091 299.11555748386712139 ) rock4_2 9.72570991516113281 95.0894012451171875 350.1099853515625 0.99535101652145386 0.97052502632141113
( 392.51561748944976671 -1758.13841025977330901 221.93166373893632226 ) ( 1199.73437537143649934 -1850.52292721460958091 299.11555748386712139 ) ( 404.63242807195160822 -1696.09174007488900315 211.96202895796943722 ) rock4_2 -2.58533000946044922 7.69421005249023438 349.858001708984375 0.99317502975463867 0.99086099863052368
( 392.51561748944976671 -1758.13841025977330901 221.93166373893632226 ) ( 685.205227597987232 -1880.05386294480922516 267.14020489435648642 ) ( 1199.73437537143649934 -1850.52292721460958091 299.11555748386712139 ) rock4_2 0.29211398959159851 -1.12084996700286865 349.87799072265625 0.99334698915481567 0.98575097322463989
( 1199.73437537143649934 -1850.52292721460958091 299.11555748386712139 ) ( 1195.3323608207340385 -1812.61180985669875554 293.31661882168685906 ) ( 404.63242807195160822 -1696.09174007488900315 211.96202895796943722 ) rock4_2 -3.78198003768920898 21.7248992919921875 349.865997314453125 0.9932439923286438 0.99966299533843994
}
)";

            IO::TestParserStatus status;
            IO::NodeReader reader(data, world);

            auto nodes = reader.read(worldBounds, status);
            EXPECT_EQ(1u, nodes.size());

            Brush brush = static_cast<BrushNode*>(nodes.front())->brush();

            const std::vector<vm::vec3> vertexPositions {
                brush.findClosestVertexPosition(vm::vec3(1169.1764156206966, -1800.2961013859342, 568.79748529920892)),
                brush.findClosestVertexPosition(vm::vec3(1164.1689509627774, -1797.7259237617193, 578.31488545196294)),
                brush.findClosestVertexPosition(vm::vec3(1163.5185572994671, -1820.7940760208414, 554.17919392904093)),
                brush.findClosestVertexPosition(vm::vec3(1120.5128684458623, -1855.3192739534061, 574.53563498325116))
            };

            ASSERT_TRUE(brush.canMoveVertices(worldBounds, vertexPositions, vm::vec3(16.0, 0.0, 0.0)));
            ASSERT_NO_THROW(brush.moveVertices(worldBounds, vertexPositions, vm::vec3(16.0, 0.0, 0.0)));
            
            kdl::col_delete_all(nodes);
        }


        TEST_CASE("BrushTest.removeVertexWithCorrectTextures_2082", "[BrushTest]") {
            // see https://github.com/kduske/TrenchBroom/issues/2082

            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(Model::MapFormat::Valve);

            const std::string data = R"(
{
( 32 -32 -0 ) ( -16 -32 -0 ) ( -16 -32 32 ) *04water1 [ -1 0 0 -0.941193 ] [ 0 0 -1 -0 ] 125.468 1 1
( -16 -32 32 ) ( -16 -32 -0 ) ( -32 -16 -0 ) *04mwat2 [ -1 0 0 -0.941193 ] [ 0 0 -1 -0 ] 125.468 1 1
( 32 32 -0 ) ( 32 -32 -0 ) ( 32 -32 32 ) *04water2 [ -2.22045e-16 -1 0 -24.9412 ] [ 0 0 -1 -0 ] 125.468 1 1
( 32 -32 32 ) ( -16 -32 32 ) ( 32 -0 64 ) *teleport [ 0 0 -1 -0 ] [ 1 0 0 0.999969 ] 270 1 1
( 32 -0 64 ) ( -16 -32 32 ) ( -32 -16 32 ) *slime1 [ 0 -1 -2.22045e-16 -0 ] [ 1 0 0 0.999969 ] 270 1 1
( 32 32 -0 ) ( -16 32 -0 ) ( -32 -16 -0 ) *lava1 [ 1 0 0 -0 ] [ 0 -1 0 0.999998 ] -0 1 1
( 32 -0 64 ) ( -16 32 32 ) ( 32 32 32 ) *slime [ 0 -1 2.22045e-16 -0 ] [ 1 0 0 0.999969 ] 270 1 1
( 32 32 32 ) ( -16 32 32 ) ( -16 32 -0 ) *04awater1 [ 0.894427 -0.447214 0 18.9966 ] [ 0 0 -1 -0 ] -0 1 1
( -16 32 -0 ) ( -16 32 32 ) ( -32 -16 32 ) *04mwat1 [ -2.22045e-16 1 0 39.0588 ] [ 0 0 -1 -0 ] 125.468 1 1
( -32 -16 32 ) ( -16 32 32 ) ( 32 -0 64 ) *slime0 [ -2.43359e-08 -1 0 0.999985 ] [ -1 2.43359e-08 0 -0 ] 90 1 1
}
)";

            IO::TestParserStatus status;
            IO::NodeReader reader(data, world);

            std::vector<Node*> nodes = reader.read(worldBounds, status);
            ASSERT_EQ(1u, nodes.size());

            Brush brush = static_cast<BrushNode*>(nodes.front())->brush();

            const vm::vec3 p1(32.0, 32.0, 0.0);
            const vm::vec3 p2(-16.0, 32.0, 0.0);
            const vm::vec3 p3(-32.0, -16.0, 0.0);
            const vm::vec3 p4(-16.0, -32.0, 0.0);
            const vm::vec3 p5(32.0, -32.0, 0.0);

            const vm::vec3 p6(32.0, 32.0, 32.0);
            const vm::vec3 p7(-16.0, 32.0, 32.0); // this vertex will be deleted
            const vm::vec3 p8(-32.0, -16.0, 32.0);
            const vm::vec3 p9(-16.0, -32.0, 32.0);
            const vm::vec3 p10(32.0, -32.0, 32.0);

            const vm::vec3 p11(32.0, 0.0, 64.0);

            // Make sure that the faces have the textures we expect before the vertex is deleted.

            // side faces
            assertTexture("*04awater1", brush, std::vector<vm::vec3d>{p1, p2, p7, p6});
            assertTexture("*04mwat1", brush, std::vector<vm::vec3d>{p2, p3, p8, p7});
            assertTexture("*04mwat2", brush, std::vector<vm::vec3d>{p3, p4, p9, p8});
            assertTexture("*04water1", brush, std::vector<vm::vec3d>{p4, p5, p10, p9});
            assertTexture("*04water2", brush, std::vector<vm::vec3d>{p5, p1, p6, p11, p10});

            // bottom face
            assertTexture("*lava1", brush, std::vector<vm::vec3d>{p5, p4, p3, p2, p1});

            // top faces
            assertTexture("*slime", brush, std::vector<vm::vec3d>{p6, p7, p11});
            assertTexture("*slime0", brush, std::vector<vm::vec3d>{p7, p8, p11});
            assertTexture("*slime1", brush, std::vector<vm::vec3d>{p8, p9, p11});
            assertTexture("*teleport", brush, std::vector<vm::vec3d>{p9, p10, p11});

            // delete the vertex
            ASSERT_TRUE(brush.canRemoveVertices(worldBounds, std::vector<vm::vec3d>{p7}));
            brush.removeVertices(worldBounds, std::vector<vm::vec3d>{p7});

            // assert the structure and textures

            // side faces
            assertTexture("*04awater1", brush, std::vector<vm::vec3d>{p1, p2, p6});
            assertTexture("*04mwat1", brush, std::vector<vm::vec3d>{p2, p3, p8});
            assertTexture("*04mwat2", brush, std::vector<vm::vec3d>{p3, p4, p9, p8});
            assertTexture("*04water1", brush, std::vector<vm::vec3d>{p4, p5, p10, p9});
            assertTexture("*04water2", brush, std::vector<vm::vec3d>{p5, p1, p6, p11, p10});

            // bottom face
            assertTexture("*lava1", brush, std::vector<vm::vec3d>{p5, p4, p3, p2, p1});

            // top faces
            assertTexture("*slime", brush, std::vector<vm::vec3d>{p6, p2, p11});
            assertTexture("*slime0", brush, std::vector<vm::vec3d>{p2, p8, p11});
            assertTexture("*slime1", brush, std::vector<vm::vec3d>{p8, p9, p11}); // failure, becomes *slime0
            assertTexture("*teleport", brush, std::vector<vm::vec3d>{p9, p10, p11});

            kdl::col_delete_all(nodes);
        }

        TEST_CASE("BrushTest.snapIssue1198", "[BrushTest]") {
            // https://github.com/kduske/TrenchBroom/issues/1198
            const std::string data("{\n"
                              "( 167.63423 -46.88446 472.36551 ) ( 66.06285 -1.98675 573.93711 ) ( 139.12681 -168.36963 500.87299 ) rock_1736 -158 527 166.79401 0.97488 -0.85268 //TX1\n"
                              "( 208 -298.77704 309.53674 ) ( 208 -283.89740 159.77713 ) ( 208 -425.90924 294.65701 ) rock_1736 -261 -291 186.67561 1 1.17558 //TX1\n"
                              "( -495.37965 -970.19919 2420.40004 ) ( -369.12126 -979.60987 2439.22145 ) ( -516.42274 -1026.66357 2533.32892 ) skill_ground -2752 -44 100.55540 0.89744 -0.99664 //TX1\n"
                              "( 208 -103.52284 489.43151 ) ( 208 -63.04567 610.86296 ) ( 80 -103.52284 489.43151 ) rock_1736 208 516 0 -1 0.94868 //TX1\n"
                              "( -450.79344 -2050.77028 440.48261 ) ( -333.56544 -2071.81325 487.37381 ) ( -470.33140 -2177.02858 432.66743 ) skill_ground -2100 -142 261.20348 0.99813 0.93021 //TX1\n"
                              "( -192.25073 -2050.77026 159.49851 ) ( -135.78626 -2071.81323 272.42748 ) ( -201.66146 -2177.02856 140.67705 ) skill_ground -2010 513 188.47871 0.99729 -0.89685 //TX1\n"
                              "( 181.06874 -76.56186 495.11416 ) ( 172.37248 -56.19832 621.18438 ) ( 63.35341 -126.83229 495.11416 ) rock_1736 197 503 0 -0.91965 0.98492 //TX1\n"
                              "( 171.46251 -48.09583 474.98238 ) ( 129.03154 -21.91225 616.98017 ) ( 105.41315 -157.70143 477.82758 ) rock_1736 -71 425 178.51302 0.85658 -1.11429 //TX1\n"
                              "( -37.21422 -6.81390 22.01408 ) ( -12.34518 -24.34492 146.34503 ) ( -92.55376 -122.11616 16.82534 ) skill_ground -6 23 182.57664 0.90171 -0.97651 //TX1\n"
                              "( -975.92228 -1778.45799 1072.52401 ) ( -911.46425 -1772.13654 1182.92865 ) ( -1036.18913 -1883.59588 1113.72975 ) skill_ground -2320 426 158.59875 0.88222 -0.82108 //TX1\n"
                              "( -984.28431 -1006.06166 2136.35663 ) ( -881.58265 -976.76783 2206.91312 ) ( -1039.55007 -1059.19179 2238.85958 ) skill_ground -2580 152 118.33189 0.90978 -0.96784 //TX1\n"
                              "( -495.37960 -2050.77026 672 ) ( -369.12118 -2071.81323 672 ) ( -516.42263 -2177.02856 672 ) skill_ground -2104 -151 260.53769 1 1 //TX1\n"
                              "( 0 -192 512 ) ( 0 -192 640 ) ( 128 -192 512 ) skill_ground 0 512 0 1 1 //TX1\n"
                              "( 0 0 512 ) ( 0 -128 512 ) ( 128 0 512 ) skill_ground 0 0 0 1 -1 //TX1\n"
                              "}");
            assertSnapToInteger(data);
        }

        TEST_CASE("BrushTest.snapIssue1202", "[BrushTest]") {
            // https://github.com/kduske/TrenchBroom/issues/1202
            const std::string data("{\n"
                              "( -384 -1440 416 ) ( -384 -1440 544 ) ( -512 -1440 416 ) skip -384 416 0 -1 1 //TX1\n"
                              "( -479.20200 -1152 448 ) ( -388.69232 -1242.50967 448 ) ( -607.20203 -1152 448 ) skip -476 1631 -45 1 -0.70711 //TX2\n"
                              "( -202.75913 -1259.70123 365.61488 ) ( -293.26877 -1169.19156 365.61487 ) ( -288.09239 -1345.03450 408.28175 ) city6_8 747 1097 135 1 0.94281 //TX2\n"
                              "( -672 -1664 112 ) ( -800 -1664 112 ) ( -672 -1664 240 ) bricka2_4 -672 112 0 -1 1 //TX2\n"
                              "( -166.47095 -1535.24850 432 ) ( -294.41554 -1539.01482 432 ) ( -38.47095 -1663.24847 432 ) bricka2_4 -212 1487 181.68613 1 1.02899 //TX2\n"
                              "( 96 -2840.62573 176 ) ( 96 -3021.64502 176 ) ( 96 -2840.62573 304 ) bricka2_4 -2009 176 0 -1.41421 1 //TX2\n"
                              "( -128 -288 176 ) ( -128 -160 176 ) ( -128 -288 304 ) bricka2_4 288 176 0 1 1 //TX2\n"
                              "}");
            assertSnapToInteger(data);
        }

        TEST_CASE("BrushTest.snapIssue1203", "[BrushTest]") {
            // https://github.com/kduske/TrenchBroom/issues/1203
            const std::string data("{\n"
                              "( -2255.07542 -1621.75354 1184 ) ( -2340.26373 -1524.09826 1184 ) ( -2255.07542 -1621.75354 1312 ) metal5_6 2126 1184 0 0.76293 1 //TX2\n"
                              "( -2274.59294 -1572.67199 1077.14252 ) ( -2216.18139 -1643.55025 1214.27523 ) ( -2179.93925 -1486.72565 1086.37772 ) metal1_2 -86 -3857 66.92847 1.16449 -0.65206 //TX2\n"
                              "( -2294.68465 -1559.17687 1145.06418 ) ( -2209.49633 -1656.83209 1145.06409 ) ( -2226.47948 -1499.67881 1009.29941 ) metal1_2 -2044 -1080 180.00005 0.76293 1.06066 //TX2\n"
                              "( -2277.90664 -1569.35830 1229.87757 ) ( -2219.49502 -1640.23662 1092.74492 ) ( -2183.25294 -1483.41196 1220.64238 ) metal1_2 1738 -2475 -66.92843 1.16449 0.65206 //TX2\n"
                              "( -2291.16152 -1556.10351 1161.99537 ) ( -2205.97305 -1653.75857 1161.99532 ) ( -2222.95604 -1496.60517 1297.75964 ) metal1_2 -2040 1096 180.00003 0.76293 -1.06066 //TX2\n"
                              "( -2081.99036 -1805.83188 1184 ) ( -2022.45370 -1920.93607 1184 ) ( -2195.68224 -1864.63800 1184 ) skinsore -640 2679 -62.65012 1.01242 -1 //TX2\n"
                              "( -2243.07853 -1621.15697 1184 ) ( -2243.07799 -1621.15750 1312 ) ( -2152.56935 -1530.64682 1184 ) metal5_6 2293 1184 0 0.70711 1 //TX1\n"
                              "( -2288.33311 -1643.78464 1184 ) ( -2197.82344 -1553.27497 1184 ) ( -2288.33311 -1643.78464 1312 ) metal5_6 2325 1184 0 0.70711 1 //TX2\n"
                              "( -2243.76171 -1610.43983 1184 ) ( -2243.76171 -1610.43983 1312 ) ( -2327.90482 -1513.98290 1184 ) metal5_6 2137 1184 0 0.75357 1 //TX1\n"
                              "}");
            assertSnapToInteger(data);
        }

        TEST_CASE("BrushTest.snapIssue1205", "[BrushTest]") {
            // https://github.com/kduske/TrenchBroom/issues/1205
            const std::string data("{\n"
                              "( 304 -895.52890 1232 ) ( 304 -763.64662 1232 ) ( 304 -895.52890 1104 ) bookshelf1w 1232 -869 -90 1 1.03033 //TX1\n"
                              "( -23.76447 -759.76453 1232 ) ( 69.49032 -666.50962 1232 ) ( -23.76447 -759.76453 1104 ) bookshelf1w 1232 -1043 -90 1 0.72855 //TX1\n"
                              "( -139.64675 -480 1232 ) ( -7.76448 -480 1232 ) ( -139.64675 -480 1104 ) bookshelf1w 1232 -136 -90 1 1.03033 //TX1\n"
                              "( -42.50967 -245.49033 1232 ) ( 50.74518 -338.74518 1232 ) ( -42.50967 -245.49033 1104 ) bookshelf1w 1232 337 -90 1 -0.72855 //TX1\n"
                              "( 323.88225 -320 1232 ) ( 191.99998 -320 1232 ) ( 323.88225 -320 1104 ) bookshelf1w 1232 -314 -90 1 -1.03033 //TX1\n"
                              "( 144 -168.23550 1232 ) ( 144 -300.11777 1232 ) ( 144 -168.23550 1104 ) bookshelf1w 1232 163 -90 1 -1.03033 //TX1\n"
                              "( 303.99988 -432.00012 1248.00050 ) ( 278.89702 -432.00012 1373.51482 ) ( 303.99988 -304.00012 1248.00050 ) rfslte1 432 1273 0 1 0.98058 //TX1\n"
                              "( 303.99995 -367.99981 1248 ) ( 286.42119 -385.57861 1373.56263 ) ( 213.49015 -277.49027 1248 ) rfslte1 430 1272 0 -0.70711 0.98096 //TX1\n"
                              "( 256 -320 1247.99999 ) ( 256 -345.10286 1373.51432 ) ( 128 -320.00005 1247.99999 ) rfslte1 256 1273 0 -1 0.98058 //TX1\n"
                              "( 191.99988 -320.00012 1248.00049 ) ( 209.57867 -337.57891 1373.56311 ) ( 101.49021 -410.50979 1248.00049 ) rfslte1 -453 1272 0 -0.70711 0.98096 //TX1\n"
                              "( 144 -368 1248.00049 ) ( 169.10289 -368 1373.51481 ) ( 144 -496 1248.00049 ) rfslte1 -368 1273 0 -1 0.98058 //TX1\n"
                              "( 144 -432 1248.00049 ) ( 161.57879 -414.42121 1373.56311 ) ( 234.50967 -522.50967 1248.00049 ) rfslte1 -611 1272 0 -0.70711 0.98096 //TX1\n"
                              "( 192 -480 1248.00049 ) ( 192 -454.89711 1373.51481 ) ( 320 -480 1248.00049 ) rfslte1 -192 1273 0 1 0.98058 //TX1\n"
                              "( 256 -480 1248.00049 ) ( 238.42121 -462.42121 1373.56311 ) ( 346.50967 -389.49033 1248.00049 ) rfslte1 679 1272 0 0.70711 0.98096 //TX1\n"
                              "( 144 -320 1232 ) ( 144 -448 1232 ) ( 272 -320 1232 ) rfslte1 -144 320 0 1 -1 //TX1\n"
                              "( 285.25483 -226.74517 1232 ) ( 191.99999 -320.00001 1232 ) ( 285.25483 -226.74517 1104 ) bookshelf1w 1232 311 -90 1 -0.72855 //TX1\n"
                              "( 304 -368 1232 ) ( 210.74516 -274.74516 1232 ) ( 304 -368 1104 ) bookshelf1w 1232 -505 -90 1 0.72855 //TX1\n"
                              "}");
            assertSnapToInteger(data);
        }

        TEST_CASE("BrushTest.snapIssue1206", "[BrushTest]") {
            // https://github.com/kduske/TrenchBroom/issues/1206
            const std::string data("{\n"
                              "( -637.50000 1446.44631 1339.47316 ) ( -637.50000 1560.93298 1396.71649 ) ( -765.50000 1446.44631 1339.47316 ) column01_3 -638 1617 0 -1 0.89443 //TX1\n"
                              "( -632.50000 1438.33507 1340.33194 ) ( -632.50000 1538.28627 1260.37098 ) ( -760.50000 1438.33507 1340.33194 ) column01_3 -632 1842 0 -1 0.78087 //TX1\n"
                              "( -646 1397.33116 1362.08442 ) ( -646 1511.81782 1304.84109 ) ( -518 1397.33116 1362.08442 ) column01_3 646 1562 0 1 0.89443 //TX1\n"
                              "( -637.50000 1436 1338 ) ( -637.50000 1436 1466 ) ( -637.50000 1308 1338 ) column01_3 1436 1338 0 -1 1 //TX1\n"
                              "( -637 1438.91806 1338.87292 ) ( -637 1367.91644 1445.37534 ) ( -509 1438.91806 1338.87292 ) column01_3 637 1609 0 1 0.83205 //TX1\n"
                              "( -637 1440.50000 1338 ) ( -637 1440.50000 1466 ) ( -637 1568.50000 1338 ) column01_3 -1440 1338 0 1 1 //TX1\n"
                              "( -638 1435.27452 1340.35014 ) ( -638 1312.19946 1375.51444 ) ( -510 1435.27452 1340.35014 ) column01_3 638 -1493 0 1 -0.96152 //TX1\n"
                              "}");
            assertSnapToInteger(data);
        }

        TEST_CASE("BrushTest.snapIssue1207", "[BrushTest]") {
            // https://github.com/kduske/TrenchBroom/issues/1207
            const std::string data("{\n"
                              "( -635.50000 1442.50000 1353.50012 ) ( -763.50000 1442.50000 1353.50012 ) ( -635.50000 1314.50000 1353.50012 ) column01_3 1442 635 -90 1 -1 //TX1\n"
                              "( -635.50000 1442.50000 1355 ) ( -507.50000 1442.50000 1355 ) ( -635.50000 1314.50000 1355 ) column01_3 1442 -635 -90 1 1 //TX1\n"
                              "( -636 1442.50000 1354 ) ( -636 1442.50000 1482 ) ( -764 1442.50000 1354 ) column01_3 -636 1354 0 -1 1 //TX1\n"
                              "( -636 1438 1354 ) ( -636 1438 1482 ) ( -636 1310 1354 ) column01_3 1438 1354 0 -1 1 //TX1\n"
                              "( -635.50000 1438 1354 ) ( -635.50000 1438 1482 ) ( -507.50000 1438 1354 ) column01_3 636 1354 0 1 1 //TX1\n"
                              "( -635.50000 1442.50000 1354 ) ( -635.50000 1442.50000 1482 ) ( -635.50000 1570.50000 1354 ) column01_3 -1442 1354 0 1 1 //TX1\n"
                              "}\n");
            assertCannotSnap(data);
        }

        TEST_CASE("BrushTest.snapIssue1232", "[BrushTest]") {
            // https://github.com/kduske/TrenchBroom/issues/1232
            const std::string data("{\n"
                              "  ( 2152.22540 381.27455 2072 ) ( 2152.22540 381.27455 2200 ) ( 2020.34268 513.15633 2072 ) wbord05 2089 2072 0 -1.03033 1 //TX1\n"
                              "  ( 2042 335.61771 2072 ) ( 2042 335.61771 2200 ) ( 2042 522.12738 2072 ) wbord05 -230 2072 0 1.45711 1 //TX1\n"
                              "  ( 1948.74515 374.24515 2072 ) ( 1948.74515 374.24515 2200 ) ( 2080.62741 506.12741 2072 ) wbord05 -363 2072 0 1.03033 1 //TX1\n"
                              "  ( 1916.74515 451.50000 2072 ) ( 1916.74515 451.50000 2200 ) ( 2103.25482 451.50000 2072 ) wbord05 -1315 2072 0 1.45711 1 //TX1\n"
                              "  ( 2043.56919 493.06919 2026.43074 ) ( 1969.66841 419.16841 2100.33167 ) ( 2134.07889 402.55957 2026.43079 ) kjwall2 -1096 -2197 -44.99997 1 -0.81650 //TX1\n"
                              "  ( 2028.72645 441.39868 2036.31307 ) ( 2140.35950 385.25273 2064.05640 ) ( 2063.24398 543.87358 2104.80712 ) kjwall2 -1262 1843 71.38448 0.84478 -0.96653 //TX1\n"
                              "  ( 1980.74480 497.22377 2022.51040 ) ( 2011.04246 392.71223 2089.91507 ) ( 2093.59579 549.47972 2052.80842 ) kjwall2 -2065 453 24.84662 0.97158 -0.84038 //TX1\n"
                              "  ( 2026.09563 451.97825 2028.19126 ) ( 1995.79798 556.48977 2095.59597 ) ( 1913.24475 399.72220 2058.48949 ) kjwall2 2088 -525 204.84669 0.97158 -0.84038 //TX1\n"
                              "  ( 1994 515.89878 2035.80067 ) ( 1994 401.41210 2093.04401 ) ( 2122 515.89859 2035.80028 ) kjwall2 -1994 -577 -0.00009 1 -0.89443 //TX1\n"
                              "  ( 2010 443.10126 2035.80060 ) ( 2010 557.58793 2093.04394 ) ( 1881.99999 443.10145 2035.80021 ) kjwall2 2010 495 179.99991 1 -0.89443 //TX1\n"
                              "  ( 2018.70638 436.61696 2056.35332 ) ( 2119.11026 375.11218 2106.55513 ) ( 2073.71821 548.87185 2083.85853 ) kjwall2 -1311 1770 63.89229 0.97664 -0.91582 //TX1\n"
                              "  ( 2034 453.83437 2044 ) ( 1982.79994 568.32105 2069.59989 ) ( 1931.59947 396.59103 2095.19895 ) kjwall2 2179 -611 209.20580 0.91652 -0.97590 //TX1\n"
                              "  ( 2018 507.50000 2072 ) ( 2018 507.50000 2200 ) ( 1831.49033 507.50000 2072 ) wbord05 1385 2072 0 -1.45711 1 //TX1\n"
                              "  ( 1986 530.12743 2072 ) ( 1986 530.12743 2200 ) ( 1986 343.61775 2072 ) wbord05 364 2072 0 -1.45711 1 //TX1\n"
                              "  ( 2010 479.50000 2072 ) ( 2010 607.50000 2072 ) ( 2138 479.50000 2072 ) kjwall2 -2010 480 0 1 1 //TX1\n"
                              "  ( 2010 479.50000 2060 ) ( 2010 351.50000 2060 ) ( 2138 479.50000 2060 ) kjwall2 -2010 -480 0 1 -1 //TX1\n"
                              "  ( 2013.31371 518.81371 2072 ) ( 2013.31371 518.81371 2200 ) ( 1881.43146 386.93146 2072 ) wbord05 504 2072 0 -1.03033 1 //TX1\n"
                              "  ( 1941.71572 511.78427 2072 ) ( 1941.71572 511.78427 2200 ) ( 2073.59785 379.90191 2072 ) wbord05 497 2072 0 -1.03033 1 //TX1\n"
                              " }\n");

            assertSnapToInteger(data);
        }

        TEST_CASE("BrushTest.snapIssue1395_24202", "[BrushTest]") {
            // https://github.com/kduske/TrenchBroom/issues/1395 brush at line 24202
            const std::string data("{\n"
                              "( -4 -325 952 ) ( -16 -356 1032 ) ( -44 -309 1016 ) rock3_8 -1.28601 -6.46194 113.395 0.943603 1.06043\n"
                              "( -17.57635498046875 -263.510009765625 988.9852294921875 ) ( -137.5655517578125 -375.941162109375 743.296875 ) ( 34.708740234375 -300.228759765625 1073.855712890625 ) rock3_8 -1.28595 -6.46191 113.395 0.943603 1.06043\n"
                              "( -135.7427978515625 -370.1265869140625 739.753173828125 ) ( -15.768181800842285 -257.6954345703125 985.42547607421875 ) ( -449.98324584960937 -364.254638671875 589.064697265625 ) rock3_8 -26.8653 -10.137 25.6205 1.15394 -1\n"
                              "( -399.50726318359375 -406.7877197265625 677.47894287109375 ) ( -137.5655517578125 -375.941162109375 743.296875 ) ( -451.79229736328125 -370.0692138671875 592.6083984375 ) rock3_8 26.1202 -7.68527 81.5004 0.875611 -1\n"
                              "( -280.1622314453125 -291.92608642578125 924.623779296875 ) ( -18.227519989013672 -261.07952880859375 990.43829345703125 ) ( -227.88420104980469 -328.64483642578125 1009.49853515625 ) rock3_8 -28.9783 0.638519 81.5019 0.875609 -1\n"
                              "( -195.9036865234375 -282.3568115234375 876.8590087890625 ) ( -143.6192626953125 -319.08740234375 961.7213134765625 ) ( -368.19818115234375 -358.08740234375 546.27716064453125 ) rock3_8 -25.9692 -19.1265 113.395 0.943603 1.06043\n"
                              "( -276.88287353515625 -332.21014404296875 930.47674560546875 ) ( -449.17929077148437 -407.92318725585937 599.90850830078125 ) ( -14.952971458435059 -301.37832641601562 996.28533935546875 ) rock3_8 -20.4888 -8.56413 -87.0938 1.30373 1.02112\n"
                              "( 37.161830902099609 -335.35406494140625 1080.605712890625 ) ( -135.12174987792969 -411.084716796875 750.062744140625 ) ( -224.79318237304687 -366.23345947265625 1014.8262329101562 ) rock3_8 8.91101 4.43578 -87.0938 1.30373 1.02112\n"
                              "( -290.354736328125 -397.304931640625 703.53790283203125 ) ( -470.618896484375 -265.4686279296875 632.53790283203125 ) ( -400.5767822265625 -391.6395263671875 703.53790283203125 ) rock3_8 8.25781 -11.1122 -165 0.865994 1\n"
                              "( -96 -299 1019 ) ( -96 -171 1019 ) ( 50 -400 1017 ) rock3_8 -28.9783 0.638519 81.5019 0.875609 -1\n"
                              "}\n");

            assertSnapToInteger(data);
        }

        TEST_CASE("BrushTest.snapIssue1395_18995", "[BrushTest]") {
            // https://github.com/kduske/TrenchBroom/issues/1395 brush at line 24202
            const std::string data("{\n"
                              "( 335 891 680 ) ( 314 881 665 ) ( 451 826 680 ) wswamp1_2 2 0 0 1 1\n"
                              "( 450 813 671 ) ( 451 826 680 ) ( 446 807 665 ) wswamp1_2 2 0 0 1 1\n"
                              "( 451 826 680 ) ( 314 881 665 ) ( 446 807 665 ) wswamp1_2 2 0 0 1 1\n"
                              "( 446 807 665 ) ( 446 754 665 ) ( 450 813 671 ) wswamp1_2 2 0 0 1 1\n"
                              "( 446 754 680 ) ( 451 826 680 ) ( 446 754 665 ) wswamp1_2 2 0 0 1 1\n"
                              "( 313 880 680 ) ( 310 879 677 ) ( 335 891 680 ) wswamp1_2 -16 0 0 1 1\n"
                              "( 304 876 670 ) ( 312 880 665 ) ( 310 879 677 ) wswamp1_2 -16 0 0 1 1\n"
                              "( 314 881 665 ) ( 335 891 680 ) ( 310 879 677 ) wswamp1_2 -16 0 0 1 1\n"
                              "( 330 754 667 ) ( 328 754 665 ) ( 342 757 680 ) wswamp1_2 2 0 0 1 1\n"
                              "( 342 757 680 ) ( 328 754 665 ) ( 310 879 677 ) wswamp1_2 2 0 0 1 1\n"
                              "( 304 876 670 ) ( 310 879 677 ) ( 328 754 665 ) wswamp1_2 2 0 0 1 1\n"
                              "( 312 823 665 ) ( 304 876 670 ) ( 328 754 665 ) wswamp1_2 2 0 0 1 1\n"
                              "( 310.50375366210937 879.1187744140625 676.45660400390625 ) ( 313.50375366210937 880.1187744140625 679.45660400390625 ) ( 342.50375366210937 757.1187744140625 679.45660400390625 ) wswamp1_2 2 0 0 1 1\n"
                              "( 308.35256958007812 876 676.95867919921875 ) ( 316.35256958007813 823 671.95867919921875 ) ( 316.35256958007813 880 671.95867919921875 ) wswamp1_2 2 0 0 1 1\n"
                              "( 342 757 680 ) ( 446 754 680 ) ( 330 754 667 ) wswamp1_2 -16 0 0 1 1\n"
                              "( 446 754 665 ) ( 328 754 665 ) ( 446 754 680 ) wswamp1_2 -16 0 0 1 1\n"
                              "( 446 754 680 ) ( 342 757 680 ) ( 451 826 680 ) wswamp1_2 -16 -2 0 1 1\n"
                              "( 446 754 665 ) ( 446 807 665 ) ( 328 754 665 ) wswamp1_2 -16 -2 0 1 1\n"
                              "}\n"
                              "\n");

            assertSnapToInteger(data);
        }
        
        TEST_CASE("BrushTest.snapToGrid64", "[BrushTest]") {
            // https://github.com/kduske/TrenchBroom/issues/1415
            const std::string data("{\n"
                              "    ( 400 224 272 ) ( 416 272 224 ) ( 304 224 224 ) techrock 128 -0 -0 1 1\n"
                              "    ( 416 448 224 ) ( 416 272 224 ) ( 400 448 272 ) techrock 64 -0 -0 1 1\n"
                              "    ( 304 272 32 ) ( 304 832 48 ) ( 304 272 48 ) techrock 64 -0 -0 1 1\n"
                              "    ( 304 448 224 ) ( 416 448 224 ) ( 304 448 272 ) techrock 128 0 0 1 1\n"
                              "    ( 400 224 224 ) ( 304 224 224 ) ( 400 224 272 ) techrock 128 -0 -0 1 1\n"
                              "    ( 352 272 272 ) ( 400 832 272 ) ( 400 272 272 ) techrock 128 -64 -0 1 1\n"
                              "    ( 304 448 224 ) ( 304 224 224 ) ( 416 448 224 ) techrock 128 -64 0 1 1\n"
                              "}\n");

            // Seems reasonable for this to fail to snap to grid 64; it's only 48 units tall.
            // If it was able to snap, that would be OK too.
            assertCannotSnapTo(data, 64.0);
        }
    }
}
