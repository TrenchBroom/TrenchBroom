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

#include "Assets/EntityDefinition.h"
#include "Model/BezierPatch.h"
#include "Model/BrushNode.h"
#include "Model/Entity.h"
#include "Model/EntityNode.h"
#include "Model/GroupNode.h"
#include "Model/Layer.h"
#include "Model/LayerNode.h"
#include "Model/PatchNode.h"
#include "Model/WorldNode.h"
#include "View/MapDocument.h"
#include "View/MapDocumentTest.h"

#include <vecmath/bbox.h>

#include <vector>

#include "Catch2.h"

namespace TrenchBroom
{
namespace View
{
TEST_CASE_METHOD(ValveMapDocumentTest, "SetLockStateTest.lockStateChanges")
{
  auto* brushNode = createBrushNode();
  auto* entityNode = new Model::EntityNode{Model::Entity{}};
  auto* patchNode = createPatchNode();

  auto* entityNodeInGroup = new Model::EntityNode{Model::Entity{}};

  document->addNodes(
    {{document->parentForNodes(),
      {brushNode, entityNode, patchNode, entityNodeInGroup}}});
  document->deselectAll();
  document->selectNodes({entityNodeInGroup});

  auto* groupNode = document->groupSelection("group");
  document->deselectAll();

  auto* layerNode = new Model::LayerNode{Model::Layer{"layer"}};
  document->addNodes({{document->world(), {layerNode}}});

  REQUIRE_FALSE(brushNode->locked());
  REQUIRE_FALSE(entityNode->locked());
  REQUIRE_FALSE(groupNode->locked());
  REQUIRE_FALSE(patchNode->locked());

  document->lock({brushNode, entityNode, groupNode, patchNode});
  CHECK(brushNode->locked());
  CHECK(entityNode->locked());
  CHECK(groupNode->locked());
  CHECK(patchNode->locked());

  document->undoCommand();
  CHECK_FALSE(brushNode->locked());
  CHECK_FALSE(entityNode->locked());
  CHECK_FALSE(groupNode->locked());
  CHECK_FALSE(patchNode->locked());

  REQUIRE_FALSE(layerNode->locked());

  document->lock({layerNode});
  CHECK(layerNode->locked());

  document->undoCommand();
  CHECK_FALSE(layerNode->locked());
}

TEST_CASE_METHOD(ValveMapDocumentTest, "SetLockStateTest.modificationCount")
{
  auto* brushNode = createBrushNode();
  auto* entityNode = new Model::EntityNode{Model::Entity{}};
  auto* patchNode = createPatchNode();

  auto* entityNodeInGroup = new Model::EntityNode{Model::Entity{}};

  document->addNodes(
    {{document->parentForNodes(),
      {brushNode, entityNode, patchNode, entityNodeInGroup}}});
  document->deselectAll();
  document->selectNodes({entityNodeInGroup});

  auto* groupNode = document->groupSelection("group");
  document->deselectAll();

  auto* layerNode = new Model::LayerNode{Model::Layer{"layer"}};
  document->addNodes({{document->world(), {layerNode}}});

  const auto originalModificationCount = document->modificationCount();

  document->lock({brushNode, entityNode, groupNode, patchNode});
  CHECK(document->modificationCount() == originalModificationCount);

  document->undoCommand();
  CHECK(document->modificationCount() == originalModificationCount);

  document->lock({layerNode});
  CHECK(document->modificationCount() == originalModificationCount + 1u);

  document->undoCommand();
  CHECK(document->modificationCount() == originalModificationCount);
}

TEST_CASE_METHOD(ValveMapDocumentTest, "SetLockStateTest.selection")
{
  auto* selectedBrushNode = createBrushNode();
  auto* unselectedBrushNode = createBrushNode();
  auto* unlockedBrushNode = createBrushNode();

  auto* layerNode = new Model::LayerNode{Model::Layer{"layer"}};
  document->addNodes({{document->world(), {layerNode}}});

  document->addNodes({{layerNode, {unlockedBrushNode}}});
  document->addNodes(
    {{document->world()->defaultLayer(), {selectedBrushNode, unselectedBrushNode}}});

  SECTION("Node selection")
  {
    document->selectNodes({selectedBrushNode, unlockedBrushNode});

    REQUIRE_THAT(
      document->selectedNodes().nodes(),
      Catch::Matchers::UnorderedEquals(
        std::vector<Model::Node*>{selectedBrushNode, unlockedBrushNode}));
    document->lock({document->world()->defaultLayer()});
    CHECK_THAT(
      document->selectedNodes().nodes(),
      Catch::Matchers::UnorderedEquals(std::vector<Model::Node*>{unlockedBrushNode}));

    document->undoCommand();
    CHECK_THAT(
      document->selectedNodes().nodes(),
      Catch::Matchers::UnorderedEquals(
        std::vector<Model::Node*>{selectedBrushNode, unlockedBrushNode}));
  }

  SECTION("Brush face selection")
  {
    document->selectBrushFaces(
      {{selectedBrushNode, 0}, {selectedBrushNode, 1}, {unlockedBrushNode, 0}});

    REQUIRE_THAT(
      document->selectedBrushFaces(),
      Catch::Matchers::UnorderedEquals(std::vector<Model::BrushFaceHandle>{
        {selectedBrushNode, 0}, {selectedBrushNode, 1}, {unlockedBrushNode, 0}}));

    document->lock({document->world()->defaultLayer()});
    CHECK_THAT(
      document->selectedBrushFaces(),
      Catch::Matchers::UnorderedEquals(
        std::vector<Model::BrushFaceHandle>{{unlockedBrushNode, 0}}));

    document->undoCommand();
    CHECK_THAT(
      document->selectedBrushFaces(),
      Catch::Matchers::UnorderedEquals(std::vector<Model::BrushFaceHandle>{
        {selectedBrushNode, 0}, {selectedBrushNode, 1}, {unlockedBrushNode, 0}}));
  }
}
} // namespace View
} // namespace TrenchBroom
