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

#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/BrushBuilder.h"
#include "Model/WorldNode.h"

#include <vecmath/vec.h>
#include <vecmath/ray.h>

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
    }
}
