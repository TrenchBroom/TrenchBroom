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
