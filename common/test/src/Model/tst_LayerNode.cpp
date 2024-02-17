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

#include "Error.h"
#include "Model/BezierPatch.h"
#include "Model/BrushBuilder.h"
#include "Model/BrushNode.h"
#include "Model/Entity.h"
#include "Model/EntityNode.h"
#include "Model/Group.h"
#include "Model/GroupNode.h"
#include "Model/Layer.h"
#include "Model/LayerNode.h"
#include "Model/MapFormat.h"
#include "Model/PatchNode.h"
#include "Model/WorldNode.h"

#include "kdl/result.h"
#include "kdl/result_io.h"

#include "Catch2.h"

namespace TrenchBroom
{
namespace Model
{
TEST_CASE("LayerNodeTest.canAddChild")
{
  constexpr auto worldBounds = vm::bbox3d{8192.0};
  constexpr auto mapFormat = MapFormat::Quake3;

  auto worldNode = WorldNode{{}, {}, mapFormat};
  auto layerNode = LayerNode{Layer{"layer"}};
  auto groupNode = GroupNode{Group{"group"}};
  auto entityNode = EntityNode{Entity{}};
  auto brushNode =
    BrushNode{BrushBuilder{mapFormat, worldBounds}.createCube(64.0, "texture").value()};

  // clang-format off
  auto patchNode = PatchNode{BezierPatch{3, 3, {
    {0, 0, 0}, {1, 0, 1}, {2, 0, 0},
    {0, 1, 1}, {1, 1, 2}, {2, 1, 1},
    {0, 2, 0}, {1, 2, 1}, {2, 2, 0} }, "texture"}};
  // clang-format on

  CHECK_FALSE(layerNode.canAddChild(&worldNode));
  CHECK_FALSE(layerNode.canAddChild(&layerNode));
  CHECK(layerNode.canAddChild(&groupNode));
  CHECK(layerNode.canAddChild(&entityNode));
  CHECK(layerNode.canAddChild(&brushNode));
  CHECK(layerNode.canAddChild(&patchNode));
}

TEST_CASE("LayerNodeTest.canRemoveChild")
{
  constexpr auto worldBounds = vm::bbox3d{8192.0};
  constexpr auto mapFormat = MapFormat::Quake3;

  auto worldNode = WorldNode{{}, {}, mapFormat};
  auto layerNode = LayerNode{Layer{"layer"}};
  auto groupNode = GroupNode{Group{"group"}};
  auto entityNode = EntityNode{Entity{}};
  auto brushNode =
    BrushNode{BrushBuilder{mapFormat, worldBounds}.createCube(64.0, "texture").value()};

  // clang-format off
  auto patchNode = PatchNode{BezierPatch{3, 3, {
    {0, 0, 0}, {1, 0, 1}, {2, 0, 0},
    {0, 1, 1}, {1, 1, 2}, {2, 1, 1},
    {0, 2, 0}, {1, 2, 1}, {2, 2, 0} }, "texture"}};
  // clang-format on

  CHECK(layerNode.canRemoveChild(&worldNode));
  CHECK(layerNode.canRemoveChild(&layerNode));
  CHECK(layerNode.canRemoveChild(&groupNode));
  CHECK(layerNode.canRemoveChild(&entityNode));
  CHECK(layerNode.canRemoveChild(&brushNode));
  CHECK(layerNode.canRemoveChild(&patchNode));
}
} // namespace Model
} // namespace TrenchBroom
