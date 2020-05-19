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
    }
}
