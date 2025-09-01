/*
 Copyright (C) 2025 Kristian Duske

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

#include "MapFixture.h"
#include "TestFactory.h"
#include "TestUtils.h"
#include "mdl/BrushNode.h"
#include "mdl/Entity.h"
#include "mdl/EntityNode.h"
#include "mdl/GroupNode.h"
#include "mdl/Layer.h"
#include "mdl/LayerNode.h"
#include "mdl/Map.h"
#include "mdl/Map_Groups.h"
#include "mdl/Map_NodeLocking.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "mdl/PatchNode.h"
#include "mdl/WorldNode.h"

#include "Catch2.h"

namespace tb::mdl
{

TEST_CASE("Map_NodeLocking")
{
  auto fixture = MapFixture{};
  auto& map = fixture.map();
  fixture.create();

  SECTION("lockNodes")
  {

    SECTION("Layer nodes")
    {
      auto* layerNode = new LayerNode{Layer{"layer"}};
      addNodes(map, {{map.world(), {layerNode}}});

      REQUIRE_FALSE(layerNode->locked());

      lockNodes(map, {layerNode});
      CHECK(layerNode->locked());

      map.undoCommand();
      CHECK_FALSE(layerNode->locked());
    }

    SECTION("Object nodes")
    {
      auto* brushNode = createBrushNode(map);
      auto* entityNode = new EntityNode{Entity{}};
      auto* patchNode = createPatchNode();

      auto* entityNodeInGroup = new EntityNode{Entity{}};

      addNodes(
        map,
        {{parentForNodes(map), {brushNode, entityNode, patchNode, entityNodeInGroup}}});
      deselectAll(map);
      selectNodes(map, {entityNodeInGroup});

      auto* groupNode = groupSelectedNodes(map, "group");
      deselectAll(map);

      REQUIRE_FALSE(brushNode->locked());
      REQUIRE_FALSE(entityNode->locked());
      REQUIRE_FALSE(groupNode->locked());
      REQUIRE_FALSE(patchNode->locked());

      lockNodes(map, {brushNode, entityNode, groupNode, patchNode});
      CHECK(brushNode->locked());
      CHECK(entityNode->locked());
      CHECK(groupNode->locked());
      CHECK(patchNode->locked());

      map.undoCommand();
      CHECK_FALSE(brushNode->locked());
      CHECK_FALSE(entityNode->locked());
      CHECK_FALSE(groupNode->locked());
      CHECK_FALSE(patchNode->locked());
    }

    SECTION("Locking increases modification count")
    {
      auto* brushNode = createBrushNode(map);
      auto* entityNode = new EntityNode{Entity{}};
      auto* patchNode = createPatchNode();

      auto* entityNodeInGroup = new EntityNode{Entity{}};

      addNodes(
        map,
        {{parentForNodes(map), {brushNode, entityNode, patchNode, entityNodeInGroup}}});
      deselectAll(map);
      selectNodes(map, {entityNodeInGroup});

      auto* groupNode = groupSelectedNodes(map, "group");
      deselectAll(map);

      auto* layerNode = new LayerNode{Layer{"layer"}};
      addNodes(map, {{map.world(), {layerNode}}});

      const auto originalModificationCount = map.modificationCount();

      lockNodes(map, {brushNode, entityNode, groupNode, patchNode});
      CHECK(map.modificationCount() == originalModificationCount);

      map.undoCommand();
      CHECK(map.modificationCount() == originalModificationCount);

      lockNodes(map, {layerNode});
      CHECK(map.modificationCount() == originalModificationCount + 1u);

      map.undoCommand();
      CHECK(map.modificationCount() == originalModificationCount);
    }

    SECTION("Locked nodes are deselected")
    {
      auto* selectedBrushNode = createBrushNode(map);
      auto* unselectedBrushNode = createBrushNode(map);
      auto* unlockedBrushNode = createBrushNode(map);

      auto* layerNode = new LayerNode{Layer{"layer"}};
      addNodes(map, {{map.world(), {layerNode}}});

      addNodes(map, {{layerNode, {unlockedBrushNode}}});
      addNodes(
        map, {{map.world()->defaultLayer(), {selectedBrushNode, unselectedBrushNode}}});

      SECTION("Node selection")
      {
        selectNodes(map, {selectedBrushNode, unlockedBrushNode});

        REQUIRE_THAT(
          map.selection().nodes,
          Catch::UnorderedEquals(std::vector<Node*>{
            selectedBrushNode,
            unlockedBrushNode,
          }));

        lockNodes(map, {map.world()->defaultLayer()});
        CHECK_THAT(
          map.selection().nodes,
          Catch::UnorderedEquals(std::vector<Node*>{unlockedBrushNode}));

        map.undoCommand();
        CHECK_THAT(
          map.selection().nodes,
          Catch::UnorderedEquals(std::vector<Node*>{
            selectedBrushNode,
            unlockedBrushNode,
          }));
      }

      SECTION("Brush face selection")
      {
        selectBrushFaces(
          map,
          {
            {selectedBrushNode, 0},
            {selectedBrushNode, 1},
            {unlockedBrushNode, 0},
          });
        REQUIRE_THAT(
          map.selection().brushFaces,
          Catch::UnorderedEquals(std::vector<BrushFaceHandle>{
            {selectedBrushNode, 0},
            {selectedBrushNode, 1},
            {unlockedBrushNode, 0},
          }));

        lockNodes(map, {map.world()->defaultLayer()});
        CHECK_THAT(
          map.selection().brushFaces,
          Catch::UnorderedEquals(std::vector<BrushFaceHandle>{
            {unlockedBrushNode, 0},
          }));

        map.undoCommand();
        CHECK_THAT(
          map.selection().brushFaces,
          Catch::UnorderedEquals(std::vector<BrushFaceHandle>{
            {selectedBrushNode, 0},
            {selectedBrushNode, 1},
            {unlockedBrushNode, 0},
          }));
      }
    }
  }
}

} // namespace tb::mdl
