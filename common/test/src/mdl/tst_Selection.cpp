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
#include "mdl/BrushFace.h"
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
     brushEntityNode
       entityBrushNode
     otherGroupNode
       groupedEntityNode
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

  auto& brushEntityNode =
    dynamic_cast<EntityNode&>(layerNode.addChild(new EntityNode{{}}));
  auto& entityBrushNode = dynamic_cast<BrushNode&>(brushEntityNode.addChild(
    brushBuilder.createCube(64.0, "material")
      .transform([](auto brush) { return std::make_unique<BrushNode>(std::move(brush)); })
      .value()
      .release()));

  auto& otherGroupNode =
    dynamic_cast<GroupNode&>(layerNode.addChild(new GroupNode{Group{"other"}}));
  auto& groupedEntityNode =
    dynamic_cast<EntityNode&>(otherGroupNode.addChild(new EntityNode{{}}));

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

  SECTION("hasAnyBrushFaces")
  {
    CHECK_FALSE(Selection{}.hasAnyBrushFaces());
    CHECK_FALSE(makeSelection({&outerGroupNode}).hasAnyBrushFaces());
    CHECK_FALSE(makeSelection({&entityNode}).hasAnyBrushFaces());
    CHECK(makeSelection({&brushNode}).hasAnyBrushFaces());
    CHECK(makeSelection({&brushNode, &entityNode}).hasAnyBrushFaces());
    CHECK_FALSE(makeSelection({&patchNode}).hasAnyBrushFaces());
    CHECK(makeSelection({BrushFaceHandle{&brushNode, 0}}).hasAnyBrushFaces());
  }

  SECTION("allEntities")
  {
    auto selection = Selection{};
    selection.cachedAllEntities = std::vector<EntityNodeBase*>{&entityNode};
    CHECK(selection.allEntities() == std::vector<EntityNodeBase*>{&entityNode});
  }

  SECTION("allBrushes")
  {
    auto selection = Selection{};
    selection.cachedAllBrushes = std::vector<BrushNode*>{&brushNode};
    CHECK(selection.allBrushes() == std::vector<BrushNode*>{&brushNode});
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

    SECTION("allEntities")
    {
      SECTION("selection is empty")
      {
        CHECK(Selection{}.allEntities() == std::vector<EntityNodeBase*>{});
      }

      SECTION("nothing selected")
      {
        CHECK(
          computeSelection(worldNode).allEntities()
          == std::vector<EntityNodeBase*>{&worldNode});
      }

      SECTION("outer group node selected")
      {
        outerGroupNode.select();
        CHECK(
          computeSelection(worldNode).allEntities()
          == std::vector<EntityNodeBase*>{&worldNode});
      }

      SECTION("entity node selected")
      {
        entityNode.select();
        CHECK(
          computeSelection(worldNode).allEntities()
          == std::vector<EntityNodeBase*>{&entityNode});
      }

      SECTION("mixed selection")
      {
        brushNode.select();
        entityNode.select();
        CHECK(
          computeSelection(worldNode).allEntities()
          == std::vector<EntityNodeBase*>{&entityNode});
      }

      SECTION("other group selected")
      {
        otherGroupNode.select();
        CHECK(
          computeSelection(worldNode).allEntities()
          == std::vector<EntityNodeBase*>{&groupedEntityNode});
      }

      SECTION("nested entity selected")
      {
        groupedEntityNode.select();
        CHECK(
          computeSelection(worldNode).allEntities()
          == std::vector<EntityNodeBase*>{&groupedEntityNode});
      }

      SECTION("face selected")
      {
        brushNode.selectFace(0);
        CHECK(
          computeSelection(worldNode).allEntities()
          == std::vector<EntityNodeBase*>{&worldNode});
      }
    }

    SECTION("allBrushes")
    {
      SECTION("selection is empty")
      {
        CHECK(Selection{}.allBrushes() == std::vector<BrushNode*>{});
      }

      SECTION("nothing selected")
      {
        CHECK(computeSelection(worldNode).allBrushes() == std::vector<BrushNode*>{});
      }

      SECTION("outer group node selected")
      {
        outerGroupNode.select();
        CHECK(
          computeSelection(worldNode).allBrushes()
          == std::vector<BrushNode*>{&brushNode});
      }

      SECTION("entity node selected")
      {
        entityNode.select();
        CHECK(computeSelection(worldNode).allBrushes() == std::vector<BrushNode*>{});
      }

      SECTION("mixed selection")
      {
        brushNode.select();
        entityNode.select();
        CHECK(
          computeSelection(worldNode).allBrushes()
          == std::vector<BrushNode*>{&brushNode});
      }

      SECTION("entity brush selected")
      {
        entityBrushNode.select();
        CHECK(
          computeSelection(worldNode).allBrushes()
          == std::vector<BrushNode*>{&entityBrushNode});
      }

      SECTION("face selected")
      {
        brushNode.selectFace(0);
        CHECK(computeSelection(worldNode).allBrushes() == std::vector<BrushNode*>{});
      }
    }

    SECTION("allBrushFaces")
    {
      SECTION("face selected")
      {
        brushNode.selectFace(0);
        CHECK(
          computeSelection(worldNode).allBrushFaces()
          == std::vector<BrushFaceHandle>{BrushFaceHandle{&brushNode, 0}});
      }

      SECTION("brush selected")
      {
        brushNode.select();
        CHECK(computeSelection(worldNode).allBrushFaces().size() == 6);
      }

      SECTION("mixed selection")
      {
        entityNode.select();
        brushNode.select();
        CHECK(computeSelection(worldNode).allBrushFaces().size() == 6);
      }
    }
  }
}

} // namespace tb::mdl
