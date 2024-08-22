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

#include "Model/BrushBuilder.h"
#include "Model/BrushNode.h"
#include "Model/EntityNode.h"
#include "Model/GroupNode.h"
#include "Model/LayerNode.h"
#include "Model/PatchNode.h"
#include "Model/WorldNode.h"
#include "TestUtils.h"
#include "View/MapDocument.h"
#include "View/MapDocumentTest.h"

#include <cstdio>

#include "CatchUtils/Matchers.h"

#include "Catch2.h"

namespace TrenchBroom::View
{

TEST_CASE_METHOD(MapDocumentTest, "RemoveNodesTest.removeNodes")
{
  SECTION("Update linked groups")
  {
    auto* groupNode = new Model::GroupNode{Model::Group{"test"}};
    auto* brushNode = createBrushNode();

    using CreateNode = std::function<Model::Node*(const MapDocumentTest& test)>;
    CreateNode createNode = GENERATE_COPY(
      CreateNode{[](const auto&) -> Model::Node* {
        return new Model::EntityNode{Model::Entity{}};
      }},
      CreateNode{[](const auto& test) -> Model::Node* { return test.createBrushNode(); }},
      CreateNode{
        [](const auto& test) -> Model::Node* { return test.createPatchNode(); }});

    auto* nodeToRemove = createNode(*this);
    groupNode->addChildren({brushNode, nodeToRemove});
    document->addNodes({{document->parentForNodes(), {groupNode}}});

    document->selectNodes({groupNode});
    auto* linkedGroupNode = document->createLinkedDuplicate();
    document->deselectAll();

    document->removeNodes({nodeToRemove});

    CHECK(linkedGroupNode->childCount() == 1u);

    document->undoCommand();

    REQUIRE(groupNode->childCount() == 2u);
    CHECK(linkedGroupNode->childCount() == 2u);
  }
}

TEST_CASE_METHOD(MapDocumentTest, "RemoveNodesTest.removeLayer")
{
  auto* layer = new Model::LayerNode{Model::Layer{"Layer 1"}};
  document->addNodes({{document->world(), {layer}}});

  document->removeNodes({layer});
  CHECK(layer->parent() == nullptr);

  document->undoCommand();
  CHECK(layer->parent() == document->world());
}

TEST_CASE_METHOD(MapDocumentTest, "RemoveNodesTest.removeEmptyBrushEntity")
{
  auto* layer = new Model::LayerNode{Model::Layer{"Layer 1"}};
  document->addNodes({{document->world(), {layer}}});

  auto* entity = new Model::EntityNode{Model::Entity{}};
  document->addNodes({{layer, {entity}}});

  auto* brush = createBrushNode();
  document->addNodes({{entity, {brush}}});

  document->removeNodes({brush});
  CHECK(brush->parent() == nullptr);
  CHECK(entity->parent() == nullptr);

  document->undoCommand();
  CHECK(brush->parent() == entity);
  CHECK(entity->parent() == layer);
}

TEST_CASE_METHOD(MapDocumentTest, "RemoveNodesTest.removeEmptyGroup")
{
  auto* group = new Model::GroupNode{Model::Group{"group"}};
  document->addNodes({{document->parentForNodes(), {group}}});

  document->openGroup(group);

  auto* brush = createBrushNode();
  document->addNodes({{document->parentForNodes(), {brush}}});

  document->removeNodes({brush});
  CHECK(document->currentGroup() == nullptr);
  CHECK(brush->parent() == nullptr);
  CHECK(group->parent() == nullptr);

  document->undoCommand();
  CHECK(document->currentGroup() == group);
  CHECK(brush->parent() == group);
  CHECK(group->parent() == document->world()->defaultLayer());
}

TEST_CASE_METHOD(MapDocumentTest, "RemoveNodesTest.recursivelyRemoveEmptyGroups")
{
  auto* outer = new Model::GroupNode{Model::Group{"outer"}};
  document->addNodes({{document->parentForNodes(), {outer}}});

  document->openGroup(outer);

  auto* inner = new Model::GroupNode{Model::Group{"inner"}};
  document->addNodes({{document->parentForNodes(), {inner}}});

  document->openGroup(inner);

  auto* brush = createBrushNode();
  document->addNodes({{document->parentForNodes(), {brush}}});

  document->removeNodes({brush});
  CHECK(document->currentGroup() == nullptr);
  CHECK(brush->parent() == nullptr);
  CHECK(inner->parent() == nullptr);
  CHECK(outer->parent() == nullptr);

  document->undoCommand();
  CHECK(document->currentGroup() == inner);
  CHECK(brush->parent() == inner);
  CHECK(inner->parent() == outer);
  CHECK(outer->parent() == document->world()->defaultLayer());
}

TEST_CASE_METHOD(MapDocumentTest, "RemoveNodesTest.updateLinkedGroups")
{
  auto* groupNode = new Model::GroupNode{Model::Group{"outer"}};
  document->addNodes({{document->parentForNodes(), {groupNode}}});

  document->openGroup(groupNode);

  auto* entityNode1 = new Model::EntityNode{Model::Entity{}};
  auto* entityNode2 = new Model::EntityNode{Model::Entity{}};
  document->addNodes({{document->parentForNodes(), {entityNode1, entityNode2}}});

  document->closeGroup();

  document->selectNodes({groupNode});

  auto* linkedGroupNode = document->createLinkedDuplicate();
  REQUIRE(linkedGroupNode->childCount() == groupNode->childCount());

  document->deselectAll();

  document->removeNodes({entityNode2});
  CHECK(linkedGroupNode->childCount() == groupNode->childCount());

  document->undoCommand();
  CHECK(linkedGroupNode->childCount() == groupNode->childCount());

  document->redoCommand();
  CHECK(linkedGroupNode->childCount() == groupNode->childCount());
}

TEST_CASE_METHOD(MapDocumentTest, "RemoveNodesTest.updateLinkedGroupsWithRecursiveDelete")
{
  auto* outerGroupNode = new Model::GroupNode{Model::Group{"outer"}};
  document->addNodes({{document->parentForNodes(), {outerGroupNode}}});

  document->openGroup(outerGroupNode);

  auto* outerEntityNode = new Model::EntityNode{Model::Entity{}};
  auto* innerGroupNode = new Model::GroupNode{Model::Group{"inner"}};
  document->addNodes({{document->parentForNodes(), {outerEntityNode, innerGroupNode}}});

  document->openGroup(innerGroupNode);

  auto* innerEntityNode = new Model::EntityNode{Model::Entity{}};
  document->addNodes({{document->parentForNodes(), {innerEntityNode}}});

  document->closeGroup();
  document->closeGroup();

  document->selectNodes({outerGroupNode});

  auto* linkedOuterGroupNode = document->createLinkedDuplicate();
  document->deselectAll();

  REQUIRE(
    outerGroupNode->children()
    == std::vector<Model::Node*>{outerEntityNode, innerGroupNode});
  REQUIRE_THAT(*linkedOuterGroupNode, Model::MatchesNode(*outerGroupNode));

  document->removeNodes({innerEntityNode});
  REQUIRE(outerGroupNode->children() == std::vector<Model::Node*>{outerEntityNode});
  CHECK_THAT(*linkedOuterGroupNode, Model::MatchesNode(*outerGroupNode));

  document->undoCommand();
  REQUIRE(
    outerGroupNode->children()
    == std::vector<Model::Node*>{outerEntityNode, innerGroupNode});
  CHECK_THAT(*linkedOuterGroupNode, Model::MatchesNode(*outerGroupNode));

  document->redoCommand();
  REQUIRE(outerGroupNode->children() == std::vector<Model::Node*>{outerEntityNode});
  CHECK_THAT(*linkedOuterGroupNode, Model::MatchesNode(*outerGroupNode));
}

} // namespace TrenchBroom::View
