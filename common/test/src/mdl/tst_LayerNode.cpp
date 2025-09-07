/*
 Copyright (C) 2020 Kristian Duske

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

#include "mdl/BezierPatch.h"
#include "mdl/BrushBuilder.h"
#include "mdl/BrushNode.h"
#include "mdl/Entity.h"
#include "mdl/EntityNode.h"
#include "mdl/Group.h"
#include "mdl/GroupNode.h"
#include "mdl/Layer.h"
#include "mdl/LayerNode.h"
#include "mdl/MapFormat.h"
#include "mdl/PatchNode.h"
#include "mdl/WorldNode.h"

#include "kdl/result.h"

#include <catch2/catch_test_macros.hpp>

namespace tb::mdl
{

TEST_CASE("LayerNode")
{

  SECTION("canAddChild")
  {
    constexpr auto worldBounds = vm::bbox3d{8192.0};
    constexpr auto mapFormat = MapFormat::Quake3;

    auto worldNode = WorldNode{{}, {}, mapFormat};
    auto layerNode = LayerNode{Layer{"layer"}};
    auto groupNode = GroupNode{Group{"group"}};
    auto entityNode = EntityNode{Entity{}};
    auto brushNode = BrushNode{
      BrushBuilder{mapFormat, worldBounds}.createCube(64.0, "material") | kdl::value()};

    // clang-format off
  auto patchNode = PatchNode{BezierPatch{3, 3, {
    {0, 0, 0}, {1, 0, 1}, {2, 0, 0},
    {0, 1, 1}, {1, 1, 2}, {2, 1, 1},
    {0, 2, 0}, {1, 2, 1}, {2, 2, 0} }, "material"}};
    // clang-format on

    CHECK_FALSE(layerNode.canAddChild(&worldNode));
    CHECK_FALSE(layerNode.canAddChild(&layerNode));
    CHECK(layerNode.canAddChild(&groupNode));
    CHECK(layerNode.canAddChild(&entityNode));
    CHECK(layerNode.canAddChild(&brushNode));
    CHECK(layerNode.canAddChild(&patchNode));
  }

  SECTION("canRemoveChild")
  {
    constexpr auto worldBounds = vm::bbox3d{8192.0};
    constexpr auto mapFormat = MapFormat::Quake3;

    auto worldNode = WorldNode{{}, {}, mapFormat};
    auto layerNode = LayerNode{Layer{"layer"}};
    auto groupNode = GroupNode{Group{"group"}};
    auto entityNode = EntityNode{Entity{}};
    auto brushNode = BrushNode{
      BrushBuilder{mapFormat, worldBounds}.createCube(64.0, "material") | kdl::value()};

    // clang-format off
  auto patchNode = PatchNode{BezierPatch{3, 3, {
    {0, 0, 0}, {1, 0, 1}, {2, 0, 0},
    {0, 1, 1}, {1, 1, 2}, {2, 1, 1},
    {0, 2, 0}, {1, 2, 1}, {2, 2, 0} }, "material"}};
    // clang-format on

    CHECK(layerNode.canRemoveChild(&worldNode));
    CHECK(layerNode.canRemoveChild(&layerNode));
    CHECK(layerNode.canRemoveChild(&groupNode));
    CHECK(layerNode.canRemoveChild(&entityNode));
    CHECK(layerNode.canRemoveChild(&brushNode));
    CHECK(layerNode.canRemoveChild(&patchNode));
  }
}

} // namespace tb::mdl
