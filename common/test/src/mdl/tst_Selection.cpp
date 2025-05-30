/*
 Copyright (C) 2021 Kristian Duske

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

#include "TestUtils.h"
#include "mdl/BezierPatch.h"
#include "mdl/Brush.h"
#include "mdl/BrushBuilder.h"
#include "mdl/BrushFaceHandle.h"
#include "mdl/BrushNode.h"
#include "mdl/Entity.h"
#include "mdl/EntityNode.h"
#include "mdl/Group.h"
#include "mdl/GroupNode.h"
#include "mdl/LayerNode.h"
#include "mdl/PatchNode.h"
#include "mdl/Selection.h"
#include "mdl/WorldNode.h"

#include "kdl/result.h"

#include <vector>

#include "Catch2.h"

namespace tb::mdl
{
TEST_CASE("Selection")
{
  /*
   worldNode
     outerGroupNode
       innerGroupNode
         patchNode
       brushNode
     entityNode
  */

  const auto worldBounds = vm::bbox3d{8192.0};
  auto worldNode = WorldNode{{}, {}, MapFormat::Valve};
  auto brushBuilder = BrushBuilder{worldNode.mapFormat(), worldBounds};

  auto& layerNode = *worldNode.defaultLayer();

  auto& outerGroupNode =
    dynamic_cast<GroupNode&>(layerNode.addChild(new GroupNode{Group{"outer"}}));
  auto& innerGroupNode =
    dynamic_cast<GroupNode&>(outerGroupNode.addChild(new GroupNode{Group{"inner"}}));

  auto& entityNode = dynamic_cast<EntityNode&>(layerNode.addChild(new EntityNode{{}}));
  auto& brushNode = dynamic_cast<BrushNode&>(outerGroupNode.addChild(
    brushBuilder.createCube(64.0, "material")
      .transform([](auto brush) { return std::make_unique<BrushNode>(std::move(brush)); })
      .value()
      .release()));

  // clang-format off
  auto& patchNode = dynamic_cast<PatchNode&>(innerGroupNode.addChild(new PatchNode{BezierPatch{3, 3, {
    {0, 0, 0}, {1, 0, 1}, {2, 0, 0},
    {0, 1, 1}, {1, 1, 2}, {2, 1, 1},
    {0, 2, 0}, {1, 2, 1}, {2, 2, 0} }, "material"}}));
  // clang-format on

  SECTION("hasAny")
  {
    CHECK_FALSE(Selection{}.hasAny());
    CHECK(makeSelection({&outerGroupNode}).hasAny());
    CHECK(makeSelection({&entityNode}).hasAny());
    CHECK(makeSelection({&brushNode}).hasAny());
    CHECK(makeSelection({&patchNode}).hasAny());
    CHECK(makeSelection({BrushFaceHandle{&brushNode, 0}}).hasAny());
  }

  SECTION("hasNodes")
  {
    CHECK_FALSE(Selection{}.hasNodes());
    CHECK(makeSelection({&outerGroupNode}).hasNodes());
    CHECK(makeSelection({&entityNode}).hasNodes());
    CHECK(makeSelection({&brushNode}).hasNodes());
    CHECK(makeSelection({&patchNode}).hasNodes());
    CHECK_FALSE(makeSelection({BrushFaceHandle{&brushNode, 0}}).hasNodes());
  }

  SECTION("hasGroups")
  {
    CHECK_FALSE(Selection{}.hasGroups());
    CHECK(makeSelection({&outerGroupNode}).hasGroups());
    CHECK(makeSelection({&outerGroupNode, &entityNode}).hasGroups());
    CHECK_FALSE(makeSelection({&entityNode}).hasGroups());
    CHECK_FALSE(makeSelection({BrushFaceHandle{&brushNode, 0}}).hasGroups());
  }

  SECTION("hasOnlyGroups")
  {
    CHECK_FALSE(Selection{}.hasOnlyGroups());
    CHECK(makeSelection({&outerGroupNode}).hasOnlyGroups());
    CHECK_FALSE(makeSelection({&outerGroupNode, &entityNode}).hasOnlyGroups());
    CHECK_FALSE(makeSelection({&entityNode}).hasOnlyGroups());
    CHECK_FALSE(makeSelection({BrushFaceHandle{&brushNode, 0}}).hasOnlyGroups());
  }

  SECTION("hasEntities")
  {
    CHECK_FALSE(Selection{}.hasEntities());
    CHECK(makeSelection({&entityNode}).hasEntities());
    CHECK(makeSelection({&entityNode, &brushNode}).hasEntities());
    CHECK_FALSE(makeSelection({&brushNode}).hasEntities());
    CHECK_FALSE(makeSelection({BrushFaceHandle{&brushNode, 0}}).hasEntities());
  }

  SECTION("hasOnlyEntities")
  {
    CHECK_FALSE(Selection{}.hasOnlyEntities());
    CHECK(makeSelection({&entityNode}).hasOnlyEntities());
    CHECK_FALSE(makeSelection({&entityNode, &brushNode}).hasOnlyEntities());
    CHECK_FALSE(makeSelection({&brushNode}).hasOnlyEntities());
    CHECK_FALSE(makeSelection({BrushFaceHandle{&brushNode, 0}}).hasOnlyEntities());
  }

  SECTION("hasBrushes")
  {
    CHECK_FALSE(Selection{}.hasBrushes());
    CHECK(makeSelection({&brushNode}).hasBrushes());
    CHECK(makeSelection({&brushNode, &entityNode}).hasBrushes());
    CHECK_FALSE(makeSelection({&entityNode}).hasBrushes());
    CHECK_FALSE(makeSelection({BrushFaceHandle{&brushNode, 0}}).hasBrushes());
  }

  SECTION("hasOnlyBrushes")
  {
    CHECK_FALSE(Selection{}.hasOnlyBrushes());
    CHECK(makeSelection({&brushNode}).hasOnlyBrushes());
    CHECK_FALSE(makeSelection({&brushNode, &entityNode}).hasOnlyBrushes());
    CHECK_FALSE(makeSelection({&entityNode}).hasOnlyBrushes());
    CHECK_FALSE(makeSelection({BrushFaceHandle{&brushNode, 0}}).hasOnlyBrushes());
  }

  SECTION("hasPatches")
  {
    CHECK_FALSE(Selection{}.hasPatches());
    CHECK(makeSelection({&patchNode}).hasPatches());
    CHECK(makeSelection({&patchNode, &entityNode}).hasPatches());
    CHECK_FALSE(makeSelection({&entityNode}).hasPatches());
    CHECK_FALSE(makeSelection({BrushFaceHandle{&brushNode, 0}}).hasPatches());
  }

  SECTION("hasOnlyPatches")
  {
    CHECK_FALSE(Selection{}.hasOnlyPatches());
    CHECK(makeSelection({&patchNode}).hasOnlyPatches());
    CHECK_FALSE(makeSelection({&patchNode, &entityNode}).hasOnlyPatches());
    CHECK_FALSE(makeSelection({&entityNode}).hasOnlyPatches());
    CHECK_FALSE(makeSelection({BrushFaceHandle{&brushNode, 0}}).hasOnlyPatches());
  }

  SECTION("hasBrushFaces")
  {
    CHECK_FALSE(Selection{}.hasBrushFaces());
    CHECK_FALSE(makeSelection({&outerGroupNode}).hasBrushFaces());
    CHECK_FALSE(makeSelection({&entityNode}).hasBrushFaces());
    CHECK_FALSE(makeSelection({&brushNode}).hasBrushFaces());
    CHECK_FALSE(makeSelection({&patchNode}).hasBrushFaces());
    CHECK(makeSelection({BrushFaceHandle{&brushNode, 0}}).hasBrushFaces());
  }

  SECTION("computeSelection")
  {
    CHECK(computeSelection(worldNode) == Selection{});

    SECTION("face selection")
    {
      brushNode.selectFace(0);
      CHECK(
        computeSelection(worldNode) == makeSelection({BrushFaceHandle{&brushNode, 0}}));
    }

    SECTION("node selection")
    {
      brushNode.select();
      CHECK(computeSelection(worldNode) == makeSelection({&brushNode}));

      outerGroupNode.select();
      CHECK(computeSelection(worldNode) == makeSelection({&outerGroupNode, &brushNode}));

      entityNode.select();
      CHECK(
        computeSelection(worldNode)
        == makeSelection({&outerGroupNode, &brushNode, &entityNode}));
    }
  }
}

} // namespace tb::mdl
