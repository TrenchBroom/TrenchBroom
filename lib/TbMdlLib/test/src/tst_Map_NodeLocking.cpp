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

#include "mdl/BrushNode.h"
#include "mdl/CatchConfig.h"
#include "mdl/Entity.h"
#include "mdl/EntityNode.h"
#include "mdl/GroupNode.h"
#include "mdl/Layer.h"
#include "mdl/LayerNode.h"
#include "mdl/Map.h"
#include "mdl/MapFixture.h"
#include "mdl/Map_Groups.h"
#include "mdl/Map_NodeLocking.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "mdl/PatchNode.h"
#include "mdl/TestFactory.h"
#include "mdl/TestUtils.h"
#include "mdl/WorldNode.h"

#include "kd/vector_utils.h"

#include <memory>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

namespace tb::mdl
{
using namespace Catch::Matchers;

TEST_CASE("Map_NodeLocking")
{
  auto fixture = MapFixture{};
  auto& map = fixture.create();

  SECTION("lockNodes")
  {
    SECTION("Layer nodes")
    {
      auto* layerNode =
        addNode(map, map.worldNode(), std::make_unique<LayerNode>(Layer{"layer"}));

      REQUIRE_FALSE(layerNode->locked());

      lockNodes(map, {layerNode});
      CHECK(layerNode->locked());

      map.undoCommand();
      CHECK(!layerNode->locked());
    }

    SECTION("Object nodes")
    {
      auto brushNodePtr = createBrushNode(map);
      auto* brushNode = brushNodePtr.get();
      auto entityNodePtr = std::make_unique<EntityNode>(Entity{});
      auto* entityNode = entityNodePtr.get();
      auto patchNodePtr = createPatchNode();
      auto* patchNode = patchNodePtr.get();

      auto entityNodeInGroupPtr = std::make_unique<EntityNode>(Entity{});
      auto* entityNodeInGroup = entityNodeInGroupPtr.get();

      addNodes(
        map,
        parentForNodes(map),
        kdl::vec_from<std::unique_ptr<Node>>(
          std::move(brushNodePtr),
          std::move(entityNodePtr),
          std::move(patchNodePtr),
          std::move(entityNodeInGroupPtr)));
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

      SECTION("Undo and redo")
      {
        map.undoCommand();
        CHECK(!brushNode->locked());
        CHECK(!entityNode->locked());
        CHECK(!groupNode->locked());
        CHECK(!patchNode->locked());

        map.redoCommand();
        CHECK(brushNode->locked());
        CHECK(entityNode->locked());
        CHECK(groupNode->locked());
        CHECK(patchNode->locked());
      }
    }

    SECTION("Locking increases modification count")
    {
      auto brushNodePtr = createBrushNode(map);
      auto* brushNode = brushNodePtr.get();
      auto entityNodePtr = std::make_unique<EntityNode>(Entity{});
      auto* entityNode = entityNodePtr.get();
      auto patchNodePtr = createPatchNode();
      auto* patchNode = patchNodePtr.get();

      auto entityNodeInGroupPtr = std::make_unique<EntityNode>(Entity{});
      auto* entityNodeInGroup = entityNodeInGroupPtr.get();

      addNodes(
        map,
        parentForNodes(map),
        kdl::vec_from<std::unique_ptr<Node>>(
          std::move(brushNodePtr),
          std::move(entityNodePtr),
          std::move(patchNodePtr),
          std::move(entityNodeInGroupPtr)));
      deselectAll(map);
      selectNodes(map, {entityNodeInGroup});

      auto* groupNode = groupSelectedNodes(map, "group");
      deselectAll(map);

      auto* layerNode =
        addNode(map, map.worldNode(), std::make_unique<LayerNode>(Layer{"layer"}));

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
      auto selectedBrushNodePtr = createBrushNode(map);
      auto* selectedBrushNode = selectedBrushNodePtr.get();

      auto* layerNode =
        addNode(map, map.worldNode(), std::make_unique<LayerNode>(Layer{"layer"}));

      auto* unlockedBrushNode = addNode(map, *layerNode, createBrushNode(map));
      addNodes(
        map,
        *map.worldNode().defaultLayer(),
        kdl::vec_from<std::unique_ptr<Node>>(
          std::move(selectedBrushNodePtr), createBrushNode(map)));

      SECTION("Node selection")
      {
        selectNodes(map, {selectedBrushNode, unlockedBrushNode});

        REQUIRE_THAT(
          map.selection().nodes,
          UnorderedEquals(std::vector<Node*>{
            selectedBrushNode,
            unlockedBrushNode,
          }));

        lockNodes(map, {map.worldNode().defaultLayer()});
        CHECK_THAT(
          map.selection().nodes, UnorderedEquals(std::vector<Node*>{unlockedBrushNode}));

        map.undoCommand();
        CHECK_THAT(
          map.selection().nodes,
          UnorderedEquals(std::vector<Node*>{
            selectedBrushNode,
            unlockedBrushNode,
          }));
      }

      SECTION("Brush face selection")
      {
        selectBrushFaces(
          map,
          {
            {*selectedBrushNode, 0},
            {*selectedBrushNode, 1},
            {*unlockedBrushNode, 0},
          });
        REQUIRE_THAT(
          map.selection().brushFaces,
          UnorderedEquals(std::vector<BrushFaceHandle>{
            {*selectedBrushNode, 0},
            {*selectedBrushNode, 1},
            {*unlockedBrushNode, 0},
          }));

        lockNodes(map, {map.worldNode().defaultLayer()});
        CHECK_THAT(
          map.selection().brushFaces,
          UnorderedEquals(std::vector<BrushFaceHandle>{
            {*unlockedBrushNode, 0},
          }));

        map.undoCommand();
        CHECK_THAT(
          map.selection().brushFaces,
          UnorderedEquals(std::vector<BrushFaceHandle>{
            {*selectedBrushNode, 0},
            {*selectedBrushNode, 1},
            {*unlockedBrushNode, 0},
          }));
      }
    }
  }

  SECTION("unlockNodes")
  {
    const auto brushNodePtr = createBrushNode(map);
    const auto entityNodePtr = std::make_unique<EntityNode>(Entity{});
    auto* brushNode = brushNodePtr.get();
    auto* entityNode = entityNodePtr.get();

    lockNodes(map, {brushNode, entityNode});
    REQUIRE(brushNode->locked());
    REQUIRE(entityNode->locked());

    unlockNodes(map, {brushNode, entityNode});
    CHECK(!brushNode->locked());
    CHECK(!entityNode->locked());

    SECTION("Undo and redo")
    {
      map.undoCommand();
      CHECK(brushNode->locked());
      CHECK(entityNode->locked());

      map.redoCommand();
      CHECK(!brushNode->locked());
      CHECK(!entityNode->locked());
    }
  }

  SECTION("ensureNodesUnlocked")
  {
    auto brushNodePtr = createBrushNode(map);
    auto* brushNode = brushNodePtr.get();
    auto entityNodePtr = std::make_unique<EntityNode>(Entity{});
    auto* entityNode = entityNodePtr.get();
    addNodes(
      map,
      parentForNodes(map),
      kdl::vec_from<std::unique_ptr<Node>>(
        std::move(brushNodePtr), std::move(entityNodePtr)));

    lockNodes(map, {brushNode});
    REQUIRE(brushNode->locked());
    REQUIRE(!entityNode->locked());

    ensureNodesUnlocked(map, {brushNode, entityNode});
    CHECK(!brushNode->locked());
    CHECK(!entityNode->locked());

    SECTION("Undo and redo")
    {
      map.undoCommand();
      CHECK(brushNode->locked());
      CHECK(!entityNode->locked());

      map.redoCommand();
      CHECK(!brushNode->locked());
      CHECK(!entityNode->locked());
    }
  }

  SECTION("resetNodeLockingState")
  {
    auto lockedNodePtr = std::make_unique<EntityNode>(Entity{});
    auto* lockedNode = lockedNodePtr.get();
    auto unlockedNodePtr = std::make_unique<EntityNode>(Entity{});
    auto* unlockedNode = unlockedNodePtr.get();
    auto inheritedNodePtr = std::make_unique<EntityNode>(Entity{});
    auto* inheritedNode = inheritedNodePtr.get();
    addNodes(
      map,
      parentForNodes(map),
      kdl::vec_from<std::unique_ptr<Node>>(
        std::move(lockedNodePtr),
        std::move(unlockedNodePtr),
        std::move(inheritedNodePtr)));

    lockNodes(map, {lockedNode});
    unlockNodes(map, {unlockedNode});
    REQUIRE(lockedNode->lockState() == LockState::Locked);
    REQUIRE(unlockedNode->lockState() == LockState::Unlocked);
    REQUIRE(inheritedNode->lockState() == LockState::Inherited);

    resetNodeLockingState(map, {lockedNode, unlockedNode, inheritedNode});
    CHECK(lockedNode->lockState() == LockState::Inherited);
    CHECK(unlockedNode->lockState() == LockState::Inherited);
    CHECK(inheritedNode->lockState() == LockState::Inherited);

    SECTION("Undo and redo")
    {
      map.undoCommand();
      CHECK(lockedNode->lockState() == LockState::Locked);
      CHECK(unlockedNode->lockState() == LockState::Unlocked);
      CHECK(inheritedNode->lockState() == LockState::Inherited);

      map.redoCommand();
      CHECK(lockedNode->lockState() == LockState::Inherited);
      CHECK(unlockedNode->lockState() == LockState::Inherited);
      CHECK(inheritedNode->lockState() == LockState::Inherited);
    }
  }

  SECTION("downgradeUnlockedToInherit")
  {
    auto lockedNodePtr = std::make_unique<EntityNode>(Entity{});
    auto* lockedNode = lockedNodePtr.get();
    auto unlockedNodePtr = std::make_unique<EntityNode>(Entity{});
    auto* unlockedNode = unlockedNodePtr.get();
    auto inheritedNodePtr = std::make_unique<EntityNode>(Entity{});
    auto* inheritedNode = inheritedNodePtr.get();
    addNodes(
      map,
      parentForNodes(map),
      kdl::vec_from<std::unique_ptr<Node>>(
        std::move(lockedNodePtr),
        std::move(unlockedNodePtr),
        std::move(inheritedNodePtr)));

    lockNodes(map, {lockedNode});
    unlockNodes(map, {unlockedNode});
    REQUIRE(lockedNode->lockState() == LockState::Locked);
    REQUIRE(unlockedNode->lockState() == LockState::Unlocked);
    REQUIRE(inheritedNode->lockState() == LockState::Inherited);

    downgradeUnlockedToInherit(map, {lockedNode, unlockedNode, inheritedNode});
    CHECK(lockedNode->lockState() == LockState::Locked);
    CHECK(unlockedNode->lockState() == LockState::Inherited);
    CHECK(inheritedNode->lockState() == LockState::Inherited);

    SECTION("Undo and redo")
    {
      map.undoCommand();
      CHECK(lockedNode->lockState() == LockState::Locked);
      CHECK(unlockedNode->lockState() == LockState::Unlocked);
      CHECK(inheritedNode->lockState() == LockState::Inherited);

      map.redoCommand();
      CHECK(lockedNode->lockState() == LockState::Locked);
      CHECK(unlockedNode->lockState() == LockState::Inherited);
      CHECK(inheritedNode->lockState() == LockState::Inherited);
    }
  }
}

} // namespace tb::mdl
