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

#include "TestUtils.h"
#include "mdl/BezierPatch.h"
#include "mdl/Brush.h"
#include "mdl/BrushNode.h"
#include "mdl/Entity.h"
#include "mdl/EntityDefinition.h"
#include "mdl/EntityNode.h"
#include "mdl/Group.h"
#include "mdl/GroupNode.h"
#include "mdl/Material.h"
#include "mdl/MaterialManager.h"
#include "mdl/NodeContents.h"
#include "mdl/PatchNode.h"
#include "mdl/SwapNodeContentsCommand.h"
#include "ui/MapDocument.h"
#include "ui/MapDocumentTest.h"

#include <memory>

#include "Catch2.h"

namespace tb::ui
{

TEST_CASE_METHOD(MapDocumentTest, "SwapNodeContentsTest.swapBrushes")
{
  auto* brushNode = createBrushNode();
  document->addNodes({{document->parentForNodes(), {brushNode}}});

  const auto originalBrush = brushNode->brush();
  auto modifiedBrush = originalBrush;
  REQUIRE(modifiedBrush
            .transform(
              document->worldBounds(), vm::translation_matrix(vm::vec3d(16, 0, 0)), false)
            .is_success());

  auto nodesToSwap = std::vector<std::pair<mdl::Node*, mdl::NodeContents>>{};
  nodesToSwap.emplace_back(brushNode, modifiedBrush);

  document->swapNodeContents("Swap Nodes", std::move(nodesToSwap), {});
  CHECK(brushNode->brush() == modifiedBrush);

  document->undoCommand();
  CHECK(brushNode->brush() == originalBrush);
}

TEST_CASE_METHOD(MapDocumentTest, "SwapNodeContentsTest.swapPatches")
{
  auto* patchNode = createPatchNode();
  document->addNodes({{document->parentForNodes(), {patchNode}}});

  const auto originalPatch = patchNode->patch();
  auto modifiedPatch = originalPatch;
  modifiedPatch.transform(vm::translation_matrix(vm::vec3d{16, 0, 0}));

  auto nodesToSwap = std::vector<std::pair<mdl::Node*, mdl::NodeContents>>{};
  nodesToSwap.emplace_back(patchNode, modifiedPatch);

  document->swapNodeContents("Swap Nodes", std::move(nodesToSwap), {});
  CHECK(patchNode->patch() == modifiedPatch);

  document->undoCommand();
  CHECK(patchNode->patch() == originalPatch);
}

TEST_CASE_METHOD(MapDocumentTest, "SwapNodeContentsTest.materialUsageCount")
{
  document->deselectAll();
  document->setProperty(mdl::EntityPropertyKeys::Wad, "fixture/test/io/Wad/cr8_czg.wad");

  constexpr auto MaterialName = "bongs2";
  const auto* material = document->materialManager().material(MaterialName);
  REQUIRE(material != nullptr);

  auto* brushNode = createBrushNode(MaterialName);
  document->addNodes({{document->parentForNodes(), {brushNode}}});

  const auto& originalBrush = brushNode->brush();
  auto modifiedBrush = originalBrush;
  REQUIRE(modifiedBrush
            .transform(
              document->worldBounds(), vm::translation_matrix(vm::vec3d(16, 0, 0)), false)
            .is_success());

  auto nodesToSwap = std::vector<std::pair<mdl::Node*, mdl::NodeContents>>{};
  nodesToSwap.emplace_back(brushNode, std::move(modifiedBrush));

  REQUIRE(material->usageCount() == 6u);

  document->swapNodeContents("Swap Nodes", std::move(nodesToSwap), {});
  CHECK(material->usageCount() == 6u);

  document->undoCommand();
  CHECK(material->usageCount() == 6u);
}

TEST_CASE_METHOD(MapDocumentTest, "SwapNodeContentsTest.entityDefinitionUsageCount")
{
  constexpr auto Classname = "point_entity";

  auto* entityNode = new mdl::EntityNode{mdl::Entity{{
    {mdl::EntityPropertyKeys::Classname, Classname},
  }}};

  document->addNodes({{document->parentForNodes(), {entityNode}}});

  const auto& originalEntity = entityNode->entity();
  auto modifiedEntity = originalEntity;
  modifiedEntity.addOrUpdateProperty("this", "that");

  auto nodesToSwap = std::vector<std::pair<mdl::Node*, mdl::NodeContents>>{};
  nodesToSwap.emplace_back(entityNode, std::move(modifiedEntity));

  REQUIRE(m_pointEntityDef->usageCount() == 1u);

  document->swapNodeContents("Swap Nodes", std::move(nodesToSwap), {});
  CHECK(m_pointEntityDef->usageCount() == 1u);

  document->undoCommand();
  CHECK(m_pointEntityDef->usageCount() == 1u);
}

TEST_CASE_METHOD(MapDocumentTest, "SwapNodesContentCommandTest.updateLinkedGroups")
{
  auto* groupNode = new mdl::GroupNode{mdl::Group{"group"}};
  auto* brushNode = createBrushNode();
  groupNode->addChild(brushNode);
  document->addNodes({{document->parentForNodes(), {groupNode}}});

  document->selectNodes({groupNode});
  auto* linkedGroupNode = document->createLinkedDuplicate();
  document->deselectAll();

  document->selectNodes({linkedGroupNode});
  document->translate(vm::vec3d(32.0, 0.0, 0.0));
  document->deselectAll();

  const auto originalBrushBounds = brushNode->physicalBounds();

  document->selectNodes({brushNode});
  document->translate(vm::vec3d(0.0, 16.0, 0.0));

  REQUIRE(
    brushNode->physicalBounds()
    == originalBrushBounds.translate(vm::vec3d(0.0, 16.0, 0.0)));

  REQUIRE(linkedGroupNode->childCount() == 1u);
  auto* linkedBrushNode =
    dynamic_cast<mdl::BrushNode*>(linkedGroupNode->children().front());
  REQUIRE(linkedBrushNode != nullptr);

  CHECK(
    linkedBrushNode->physicalBounds()
    == brushNode->physicalBounds().transform(linkedGroupNode->group().transformation()));

  document->undoCommand();

  linkedBrushNode = dynamic_cast<mdl::BrushNode*>(linkedGroupNode->children().front());
  REQUIRE(linkedBrushNode != nullptr);

  CHECK(
    linkedBrushNode->physicalBounds()
    == brushNode->physicalBounds().transform(linkedGroupNode->group().transformation()));
}

TEST_CASE_METHOD(MapDocumentTest, "SwapNodesContentCommandTest.updateLinkedGroupsFails")
{
  auto* groupNode = new mdl::GroupNode{mdl::Group{"group"}};
  auto* brushNode = createBrushNode();
  groupNode->addChild(brushNode);
  document->addNodes({{document->parentForNodes(), {groupNode}}});

  document->selectNodes({groupNode});
  auto* linkedGroupNode = document->createLinkedDuplicate();
  document->deselectAll();

  // moving the brush in linked group node will fail because it will go out of world
  // bounds
  document->selectNodes({linkedGroupNode});
  REQUIRE(document->translate(
    document->worldBounds().max - linkedGroupNode->physicalBounds().size()));
  document->deselectAll();

  const auto originalBrushBounds = brushNode->physicalBounds();

  document->selectNodes({brushNode});
  CHECK_FALSE(document->translate(vm::vec3d(0.0, 16.0, 0.0)));

  REQUIRE(brushNode->physicalBounds() == originalBrushBounds);

  REQUIRE(linkedGroupNode->childCount() == 1u);
  auto* linkedBrushNode =
    dynamic_cast<mdl::BrushNode*>(linkedGroupNode->children().front());
  REQUIRE(linkedBrushNode != nullptr);

  CHECK(
    linkedBrushNode->physicalBounds()
    == brushNode->physicalBounds().transform(linkedGroupNode->group().transformation()));
}

} // namespace tb::ui
