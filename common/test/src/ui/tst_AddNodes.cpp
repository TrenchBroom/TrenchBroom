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

#include "mdl/BrushNode.h"
#include "mdl/Entity.h"
#include "mdl/EntityNode.h"
#include "mdl/GroupNode.h"
#include "mdl/LayerNode.h"
#include "mdl/PatchNode.h"
#include "mdl/WorldNode.h"
#include "ui/MapDocument.h"
#include "ui/MapDocumentTest.h"

#include "kdl/overload.h"

#include "Catch2.h"

namespace tb::ui
{

TEST_CASE_METHOD(MapDocumentTest, "AddNodesTest.addNodes")
{
  SECTION("Update linked groups")
  {
    auto* groupNode = new mdl::GroupNode{mdl::Group{"test"}};
    auto* brushNode = createBrushNode();
    groupNode->addChild(brushNode);
    document->addNodes({{document->parentForNodes(), {groupNode}}});

    document->selectNodes({groupNode});
    auto* linkedGroupNode = document->createLinkedDuplicate();
    document->deselectAll();

    using CreateNode = std::function<mdl::Node*(const MapDocumentTest& test)>;
    CreateNode createNode = GENERATE_COPY(
      CreateNode{
        [](const auto&) -> mdl::Node* { return new mdl::EntityNode{mdl::Entity{}}; }},
      CreateNode{[](const auto& test) -> mdl::Node* { return test.createBrushNode(); }},
      CreateNode{[](const auto& test) -> mdl::Node* { return test.createPatchNode(); }});

    auto* nodeToAdd = createNode(*this);
    document->addNodes({{groupNode, {nodeToAdd}}});

    CHECK(linkedGroupNode->childCount() == 2u);

    auto* linkedNode = linkedGroupNode->children().back();
    linkedNode->accept(kdl::overload(
      [](const mdl::WorldNode*) {},
      [](const mdl::LayerNode*) {},
      [](const mdl::GroupNode*) {},
      [&](const mdl::EntityNode* linkedEntityNode) {
        const auto* originalEntityNode = dynamic_cast<mdl::EntityNode*>(nodeToAdd);
        REQUIRE(originalEntityNode);
        CHECK(originalEntityNode->entity() == linkedEntityNode->entity());
      },
      [&](const mdl::BrushNode* linkedBrushNode) {
        const auto* originalBrushNode = dynamic_cast<mdl::BrushNode*>(nodeToAdd);
        REQUIRE(originalBrushNode);
        CHECK(originalBrushNode->brush() == linkedBrushNode->brush());
      },
      [&](const mdl::PatchNode* linkedPatchNode) {
        const auto* originalPatchNode = dynamic_cast<mdl::PatchNode*>(nodeToAdd);
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
  auto* groupNode = new mdl::GroupNode{mdl::Group{"group"}};
  document->addNodes({{document->parentForNodes(), {groupNode}}});

  document->selectNodes({groupNode});
  auto* linkedGroupNode = document->createLinkedDuplicate();
  document->deselectAll();

  document->selectNodes({linkedGroupNode});
  document->translate(vm::vec3d(32.0, 0.0, 0.0));
  document->deselectAll();

  auto* brushNode = createBrushNode();
  document->addNodes({{groupNode, {brushNode}}});

  REQUIRE(groupNode->childCount() == 1u);
  CHECK(linkedGroupNode->childCount() == 1u);

  auto* linkedBrushNode =
    dynamic_cast<mdl::BrushNode*>(linkedGroupNode->children().front());
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
  auto* groupNode = new mdl::GroupNode{mdl::Group{"group"}};
  document->addNodes({{document->parentForNodes(), {groupNode}}});

  document->selectNodes({groupNode});
  auto* linkedGroupNode = document->createLinkedDuplicate();
  document->deselectAll();

  // adding a brush to the linked group node will fail because it will go out of world
  // bounds
  document->selectNodes({linkedGroupNode});
  document->translate(document->worldBounds().max);
  document->deselectAll();

  auto* brushNode = createBrushNode();
  CHECK(document->addNodes({{groupNode, {brushNode}}}).empty());

  CHECK(groupNode->childCount() == 0u);
  CHECK(linkedGroupNode->childCount() == 0u);
}

} // namespace tb::ui
