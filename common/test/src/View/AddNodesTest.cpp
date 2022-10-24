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

#include "Model/BrushNode.h"
#include "Model/Entity.h"
#include "Model/EntityNode.h"
#include "Model/GroupNode.h"
#include "Model/LayerNode.h"
#include "Model/PatchNode.h"
#include "Model/WorldNode.h"
#include "View/MapDocument.h"
#include "View/MapDocumentTest.h"

#include <vecmath/bbox.h>
#include <vecmath/bbox_io.h>
#include <vecmath/mat.h>
#include <vecmath/mat_ext.h>
#include <vecmath/mat_io.h>
#include <vecmath/vec.h>
#include <vecmath/vec_io.h>

#include <kdl/overload.h>

#include "Catch2.h"

namespace TrenchBroom
{
namespace View
{
TEST_CASE_METHOD(MapDocumentTest, "AddNodesTest.addNodes")
{
  SECTION("Update linked groups")
  {
    auto* groupNode = new Model::GroupNode{Model::Group{"test"}};
    auto* brushNode = createBrushNode();
    groupNode->addChild(brushNode);
    document->addNodes({{document->parentForNodes(), {groupNode}}});

    document->selectNodes({groupNode});
    auto* linkedGroupNode = document->createLinkedDuplicate();
    document->deselectAll();

    using CreateNode = std::function<Model::Node*(const MapDocumentTest& test)>;
    CreateNode createNode = GENERATE_COPY(
      CreateNode{[](const auto&) -> Model::Node* {
        return new Model::EntityNode{Model::Entity{}};
      }},
      CreateNode{[](const auto& test) -> Model::Node* { return test.createBrushNode(); }},
      CreateNode{
        [](const auto& test) -> Model::Node* { return test.createPatchNode(); }});

    auto* nodeToAdd = createNode(*this);
    document->addNodes({{groupNode, {nodeToAdd}}});

    CHECK(linkedGroupNode->childCount() == 2u);

    auto* linkedNode = linkedGroupNode->children().back();
    linkedNode->accept(kdl::overload(
      [](const Model::WorldNode*) {},
      [](const Model::LayerNode*) {},
      [](const Model::GroupNode*) {},
      [&](const Model::EntityNode* linkedEntityNode) {
        const auto* originalEntityNode = dynamic_cast<Model::EntityNode*>(nodeToAdd);
        REQUIRE(originalEntityNode);
        CHECK(originalEntityNode->entity() == linkedEntityNode->entity());
      },
      [&](const Model::BrushNode* linkedBrushNode) {
        const auto* originalBrushNode = dynamic_cast<Model::BrushNode*>(nodeToAdd);
        REQUIRE(originalBrushNode);
        CHECK(originalBrushNode->brush() == linkedBrushNode->brush());
      },
      [&](const Model::PatchNode* linkedPatchNode) {
        const auto* originalPatchNode = dynamic_cast<Model::PatchNode*>(nodeToAdd);
        REQUIRE(originalPatchNode);
        CHECK(originalPatchNode->patch() == linkedPatchNode->patch());
      }));

    document->undoCommand();

    REQUIRE(groupNode->childCount() == 1u);
    CHECK(linkedGroupNode->childCount() == 1u);
  }
}

TEST_CASE_METHOD(MapDocumentTest, "AddNodesTest.updateLinkedGroups")
{
  auto* groupNode = new Model::GroupNode{Model::Group{"group"}};
  document->addNodes({{document->parentForNodes(), {groupNode}}});

  document->selectNodes({groupNode});
  auto* linkedGroupNode = document->createLinkedDuplicate();
  document->deselectAll();

  document->selectNodes({linkedGroupNode});
  document->translateObjects(vm::vec3(32.0, 0.0, 0.0));
  document->deselectAll();

  auto* brushNode = createBrushNode();
  document->addNodes({{groupNode, {brushNode}}});

  REQUIRE(groupNode->childCount() == 1u);
  CHECK(linkedGroupNode->childCount() == 1u);

  auto* linkedBrushNode =
    dynamic_cast<Model::BrushNode*>(linkedGroupNode->children().front());
  CHECK(linkedBrushNode != nullptr);

  CHECK(
    linkedBrushNode->physicalBounds()
    == brushNode->physicalBounds().transform(linkedGroupNode->group().transformation()));

  document->undoCommand();
  REQUIRE(groupNode->childCount() == 0u);
  CHECK(linkedGroupNode->childCount() == 0u);

  document->redoCommand();
  REQUIRE(groupNode->childCount() == 1u);
  CHECK(linkedGroupNode->childCount() == 1u);
}

TEST_CASE_METHOD(MapDocumentTest, "AddNodesTest.updateLinkedGroupsFails")
{
  auto* groupNode = new Model::GroupNode{Model::Group{"group"}};
  document->addNodes({{document->parentForNodes(), {groupNode}}});

  document->selectNodes({groupNode});
  auto* linkedGroupNode = document->createLinkedDuplicate();
  document->deselectAll();

  // adding a brush to the linked group node will fail because it will go out of world
  // bounds
  document->selectNodes({linkedGroupNode});
  document->translateObjects(document->worldBounds().max);
  document->deselectAll();

  auto* brushNode = createBrushNode();
  CHECK(document->addNodes({{groupNode, {brushNode}}}).empty());

  CHECK(groupNode->childCount() == 0u);
  CHECK(linkedGroupNode->childCount() == 0u);
}
} // namespace View
} // namespace TrenchBroom
