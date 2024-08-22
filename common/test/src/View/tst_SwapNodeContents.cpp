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

#include "Assets/EntityDefinition.h"
#include "Assets/Material.h"
#include "Assets/MaterialManager.h"
#include "FloatType.h"
#include "Model/BezierPatch.h"
#include "Model/Brush.h"
#include "Model/BrushNode.h"
#include "Model/Entity.h"
#include "Model/EntityNode.h"
#include "Model/Group.h"
#include "Model/GroupNode.h"
#include "Model/NodeContents.h"
#include "Model/PatchNode.h"
#include "TestUtils.h"
#include "View/MapDocument.h"
#include "View/MapDocumentTest.h"
#include "View/SwapNodeContentsCommand.h"

#include "kdl/memory_utils.h"
#include "kdl/result.h"

#include "vm/bbox.h"
#include "vm/bbox_io.h"
#include "vm/mat.h"
#include "vm/mat_ext.h"
#include "vm/mat_io.h"
#include "vm/vec.h"
#include "vm/vec_io.h"

#include <filesystem>
#include <memory>

#include "Catch2.h"

namespace TrenchBroom::View
{

TEST_CASE_METHOD(MapDocumentTest, "SwapNodeContentsTest.swapBrushes")
{
  auto* brushNode = createBrushNode();
  document->addNodes({{document->parentForNodes(), {brushNode}}});

  const auto originalBrush = brushNode->brush();
  auto modifiedBrush = originalBrush;
  REQUIRE(modifiedBrush
            .transform(
              document->worldBounds(), vm::translation_matrix(vm::vec3(16, 0, 0)), false)
            .is_success());

  auto nodesToSwap = std::vector<std::pair<Model::Node*, Model::NodeContents>>{};
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
  modifiedPatch.transform(vm::translation_matrix(vm::vec3{16, 0, 0}));

  auto nodesToSwap = std::vector<std::pair<Model::Node*, Model::NodeContents>>{};
  nodesToSwap.emplace_back(patchNode, modifiedPatch);

  document->swapNodeContents("Swap Nodes", std::move(nodesToSwap), {});
  CHECK(patchNode->patch() == modifiedPatch);

  document->undoCommand();
  CHECK(patchNode->patch() == originalPatch);
}

TEST_CASE_METHOD(MapDocumentTest, "SwapNodeContentsTest.materialUsageCount")
{
  document->deselectAll();
  document->setProperty(
    Model::EntityPropertyKeys::Wad, "fixture/test/IO/Wad/cr8_czg.wad");

  constexpr auto MaterialName = "bongs2";
  const auto* material = document->materialManager().material(MaterialName);
  REQUIRE(material != nullptr);

  auto* brushNode = createBrushNode(MaterialName);
  document->addNodes({{document->parentForNodes(), {brushNode}}});

  const auto& originalBrush = brushNode->brush();
  auto modifiedBrush = originalBrush;
  REQUIRE(modifiedBrush
            .transform(
              document->worldBounds(), vm::translation_matrix(vm::vec3(16, 0, 0)), false)
            .is_success());

  auto nodesToSwap = std::vector<std::pair<Model::Node*, Model::NodeContents>>{};
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

  auto* entityNode = new Model::EntityNode{Model::Entity{{
    {Model::EntityPropertyKeys::Classname, Classname},
  }}};

  document->addNodes({{document->parentForNodes(), {entityNode}}});

  const auto& originalEntity = entityNode->entity();
  auto modifiedEntity = originalEntity;
  modifiedEntity.addOrUpdateProperty("this", "that");

  auto nodesToSwap = std::vector<std::pair<Model::Node*, Model::NodeContents>>{};
  nodesToSwap.emplace_back(entityNode, std::move(modifiedEntity));

  REQUIRE(m_pointEntityDef->usageCount() == 1u);

  document->swapNodeContents("Swap Nodes", std::move(nodesToSwap), {});
  CHECK(m_pointEntityDef->usageCount() == 1u);

  document->undoCommand();
  CHECK(m_pointEntityDef->usageCount() == 1u);
}

TEST_CASE_METHOD(MapDocumentTest, "SwapNodesContentCommandTest.updateLinkedGroups")
{
  auto* groupNode = new Model::GroupNode{Model::Group{"group"}};
  auto* brushNode = createBrushNode();
  groupNode->addChild(brushNode);
  document->addNodes({{document->parentForNodes(), {groupNode}}});

  document->selectNodes({groupNode});
  auto* linkedGroupNode = document->createLinkedDuplicate();
  document->deselectAll();

  document->selectNodes({linkedGroupNode});
  document->translateObjects(vm::vec3(32.0, 0.0, 0.0));
  document->deselectAll();

  const auto originalBrushBounds = brushNode->physicalBounds();

  document->selectNodes({brushNode});
  document->translateObjects(vm::vec3(0.0, 16.0, 0.0));

  REQUIRE(
    brushNode->physicalBounds()
    == originalBrushBounds.translate(vm::vec3(0.0, 16.0, 0.0)));

  REQUIRE(linkedGroupNode->childCount() == 1u);
  auto* linkedBrushNode =
    dynamic_cast<Model::BrushNode*>(linkedGroupNode->children().front());
  REQUIRE(linkedBrushNode != nullptr);

  CHECK(
    linkedBrushNode->physicalBounds()
    == brushNode->physicalBounds().transform(linkedGroupNode->group().transformation()));

  document->undoCommand();

  linkedBrushNode = dynamic_cast<Model::BrushNode*>(linkedGroupNode->children().front());
  REQUIRE(linkedBrushNode != nullptr);

  CHECK(
    linkedBrushNode->physicalBounds()
    == brushNode->physicalBounds().transform(linkedGroupNode->group().transformation()));
}

TEST_CASE_METHOD(MapDocumentTest, "SwapNodesContentCommandTest.updateLinkedGroupsFails")
{
  auto* groupNode = new Model::GroupNode{Model::Group{"group"}};
  auto* brushNode = createBrushNode();
  groupNode->addChild(brushNode);
  document->addNodes({{document->parentForNodes(), {groupNode}}});

  document->selectNodes({groupNode});
  auto* linkedGroupNode = document->createLinkedDuplicate();
  document->deselectAll();

  // moving the brush in linked group node will fail because it will go out of world
  // bounds
  document->selectNodes({linkedGroupNode});
  REQUIRE(document->translateObjects(
    document->worldBounds().max - linkedGroupNode->physicalBounds().size()));
  document->deselectAll();

  const auto originalBrushBounds = brushNode->physicalBounds();

  document->selectNodes({brushNode});
  CHECK_FALSE(document->translateObjects(vm::vec3(0.0, 16.0, 0.0)));

  REQUIRE(brushNode->physicalBounds() == originalBrushBounds);

  REQUIRE(linkedGroupNode->childCount() == 1u);
  auto* linkedBrushNode =
    dynamic_cast<Model::BrushNode*>(linkedGroupNode->children().front());
  REQUIRE(linkedBrushNode != nullptr);

  CHECK(
    linkedBrushNode->physicalBounds()
    == brushNode->physicalBounds().transform(linkedGroupNode->group().transformation()));
}

} // namespace TrenchBroom::View
