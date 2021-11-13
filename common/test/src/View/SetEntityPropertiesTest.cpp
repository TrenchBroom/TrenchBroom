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
#include "Color.h"
#include "Model/BrushBuilder.h"
#include "Model/BrushNode.h"
#include "Model/Entity.h"
#include "Model/EntityNode.h"
#include "Model/GroupNode.h"
#include "Model/WorldNode.h"
#include "View/MapDocument.h"
#include "View/MapDocumentTest.h"

#include <vecmath/bbox.h>

#include <kdl/result.h>

#include <vector>

#include "TestUtils.h"

#include "Catch2.h"

namespace TrenchBroom {
namespace View {
TEST_CASE_METHOD(ValveMapDocumentTest, "SetEntityPropertiesTest.changeClassname") {
  // need to recreate these because document->setEntityDefinitions will delete the old ones
  m_pointEntityDef = new Assets::PointEntityDefinition(
    "point_entity", Color(), vm::bbox3(16.0), "this is a point entity", {}, {});

  Assets::PointEntityDefinition* largeEntityDef = new Assets::PointEntityDefinition(
    "large_entity", Color(), vm::bbox3(64.0), "this is a point entity", {}, {});
  document->setEntityDefinitions(
    std::vector<Assets::EntityDefinition*>{m_pointEntityDef, largeEntityDef});

  Model::EntityNode* entityNode = new Model::EntityNode({}, {{"classname", "large_entity"}});

  addNode(*document, document->parentForNodes(), entityNode);
  REQUIRE(entityNode->entity().definition() == largeEntityDef);

  document->deselectAll();
  document->select(entityNode);
  REQUIRE(document->selectionBounds().size() == largeEntityDef->bounds().size());

  document->setProperty("classname", "point_entity");
  CHECK(entityNode->entity().definition() == m_pointEntityDef);
  CHECK(document->selectionBounds().size() == m_pointEntityDef->bounds().size());

  document->removeProperty("classname");
  CHECK(entityNode->entity().definition() == nullptr);
  CHECK(document->selectionBounds().size() == Model::EntityNode::DefaultBounds.size());

  {
    Transaction transaction(document); // we only want to undo the following changes later
    document->setProperty("temp", "large_entity");
    document->renameProperty("temp", "classname");
    CHECK(entityNode->entity().definition() == largeEntityDef);
    CHECK(document->selectionBounds().size() == largeEntityDef->bounds().size());
  }

  document->undoCommand();
  CHECK(entityNode->entity().definition() == nullptr);
  CHECK(document->selectionBounds().size() == Model::EntityNode::DefaultBounds.size());
}

TEST_CASE_METHOD(ValveMapDocumentTest, "SetEntityPropertiesTest.setProtectedProperty") {
  auto* entityNode = new Model::EntityNode{Model::Entity{}};
  document->addNodes({{document->parentForNodes(), {entityNode}}});

  document->select(entityNode);

  SECTION("Set protected property") {
    document->setProtectedProperty("some_key", true);
    CHECK_THAT(
      entityNode->entity().protectedProperties(),
      Catch::UnorderedEquals(std::vector<std::string>{"some_key"}));

    document->undoCommand();
    CHECK_THAT(
      entityNode->entity().protectedProperties(),
      Catch::UnorderedEquals(std::vector<std::string>{}));
  }

  SECTION("Unset protected property") {
    document->setProtectedProperty("some_key", true);
    REQUIRE_THAT(
      entityNode->entity().protectedProperties(),
      Catch::UnorderedEquals(std::vector<std::string>{"some_key"}));

    // Ensure that the consecutive SwapNodeContentsCommands are not collated
    document->deselectAll();
    document->select(entityNode);

    document->setProtectedProperty("some_key", false);
    CHECK_THAT(
      entityNode->entity().protectedProperties(),
      Catch::UnorderedEquals(std::vector<std::string>{}));

    document->undoCommand();
    CHECK_THAT(
      entityNode->entity().protectedProperties(),
      Catch::UnorderedEquals(std::vector<std::string>{"some_key"}));
  }
}

TEST_CASE_METHOD(
  ValveMapDocumentTest, "SetEntityPropertiesTest.setProtectedPropertyRestoresValue") {
  auto* entityNode = new Model::EntityNode{{}, {{"some_key", "some_value"}}};
  document->addNodes({{document->parentForNodes(), {entityNode}}});

  document->select(entityNode);
  auto* groupNode = document->groupSelection("test");

  document->deselectAll();
  document->select(groupNode);

  auto* linkedGroupNode = document->createLinkedDuplicate();
  REQUIRE(linkedGroupNode->childCount() == 1u);

  // both entities have the same value initially
  auto* linkedEntityNode = dynamic_cast<Model::EntityNode*>(linkedGroupNode->children().front());
  REQUIRE(linkedEntityNode);
  REQUIRE_THAT(
    linkedEntityNode->entity().properties(),
    Catch::UnorderedEquals(std::vector<Model::EntityProperty>{{"some_key", "some_value"}}));

  document->deselectAll();
  document->select(linkedEntityNode);

  // set the property to protected in the linked entity and change its value
  document->setProtectedProperty("some_key", true);
  document->setProperty("some_key", "another_value");
  REQUIRE_THAT(
    linkedEntityNode->entity().properties(),
    Catch::UnorderedEquals(std::vector<Model::EntityProperty>{{"some_key", "another_value"}}));

  // the value in the original entity remains unchanged
  entityNode = dynamic_cast<Model::EntityNode*>(groupNode->children().front());
  REQUIRE_THAT(
    entityNode->entity().properties(),
    Catch::UnorderedEquals(std::vector<Model::EntityProperty>{{"some_key", "some_value"}}));

  SECTION("When there is an unprotected property in the corresponding entity") {
    // set the property to unprotected, now the original value should be restored
    document->setProtectedProperty("some_key", false);

    entityNode = dynamic_cast<Model::EntityNode*>(groupNode->children().front());
    CHECK_THAT(
      linkedEntityNode->entity().properties(),
      Catch::UnorderedEquals(std::vector<Model::EntityProperty>{{"some_key", "some_value"}}));
    CHECK_THAT(
      entityNode->entity().properties(),
      Catch::UnorderedEquals(std::vector<Model::EntityProperty>{{"some_key", "some_value"}}));
  }

  SECTION("When no corresponding entity with an unprotected property can be found") {
    // set the property to protected in the original entity too
    document->deselectAll();
    document->select(entityNode);
    document->setProtectedProperty("some_key", true);

    linkedEntityNode = dynamic_cast<Model::EntityNode*>(linkedGroupNode->children().front());
    REQUIRE_THAT(
      entityNode->entity().properties(),
      Catch::UnorderedEquals(std::vector<Model::EntityProperty>{{"some_key", "some_value"}}));
    REQUIRE_THAT(
      linkedEntityNode->entity().properties(),
      Catch::UnorderedEquals(std::vector<Model::EntityProperty>{{"some_key", "another_value"}}));

    document->deselectAll();
    document->select(linkedEntityNode);
    document->setProtectedProperty("some_key", false);

    entityNode = dynamic_cast<Model::EntityNode*>(groupNode->children().front());
    CHECK_THAT(
      entityNode->entity().properties(),
      Catch::UnorderedEquals(std::vector<Model::EntityProperty>{{"some_key", "some_value"}}));
    CHECK_THAT(
      linkedEntityNode->entity().properties(),
      Catch::UnorderedEquals(std::vector<Model::EntityProperty>{{"some_key", "another_value"}}));

    SECTION(
      "Setting the property to unprotected in the original entity will fetch the new value now") {
      document->deselectAll();
      document->select(entityNode);
      document->setProtectedProperty("some_key", false);

      linkedEntityNode = dynamic_cast<Model::EntityNode*>(linkedGroupNode->children().front());
      CHECK_THAT(
        entityNode->entity().properties(),
        Catch::UnorderedEquals(std::vector<Model::EntityProperty>{{"some_key", "another_value"}}));
      CHECK_THAT(
        linkedEntityNode->entity().properties(),
        Catch::UnorderedEquals(std::vector<Model::EntityProperty>{{"some_key", "another_value"}}));
    }
  }

  SECTION("When setting a property to unprotected that only exists in one entity") {
    document->setProtectedProperty("yet_another_key", true);
    document->setProperty("yet_another_key", "yet_another_value");

    entityNode = dynamic_cast<Model::EntityNode*>(groupNode->children().front());
    REQUIRE_THAT(
      entityNode->entity().properties(),
      Catch::UnorderedEquals(std::vector<Model::EntityProperty>{{"some_key", "some_value"}}));
    REQUIRE_THAT(
      linkedEntityNode->entity().properties(),
      Catch::UnorderedEquals(std::vector<Model::EntityProperty>{
        {"some_key", "another_value"},
        {"yet_another_key", "yet_another_value"},
      }));

    document->setProtectedProperty("yet_another_key", false);

    entityNode = dynamic_cast<Model::EntityNode*>(groupNode->children().front());
    CHECK_THAT(
      entityNode->entity().properties(), Catch::UnorderedEquals(std::vector<Model::EntityProperty>{
                                           {"some_key", "some_value"},
                                           {"yet_another_key", "yet_another_value"},
                                         }));
    CHECK_THAT(
      linkedEntityNode->entity().properties(),
      Catch::UnorderedEquals(std::vector<Model::EntityProperty>{
        {"some_key", "another_value"},
        {"yet_another_key", "yet_another_value"},
      }));
  }
}

TEST_CASE_METHOD(ValveMapDocumentTest, "SetEntityPropertiesTest.clearProtectedProperties") {
  auto* entityNode =
    new Model::EntityNode{{}, {{"some_key", "some_value"}, {"another_key", "another_value"}}};
  document->addNodes({{document->parentForNodes(), {entityNode}}});

  CHECK_FALSE(document->canClearProtectedProperties());

  document->select(entityNode);
  CHECK(document->canClearProtectedProperties());

  auto* groupNode = document->groupSelection("test");

  document->deselectAll();
  document->select(groupNode);
  CHECK(document->canClearProtectedProperties());

  auto* linkedGroupNode = document->createLinkedDuplicate();
  REQUIRE(linkedGroupNode->childCount() == 1u);

  // both entities have the same values initially
  auto* linkedEntityNode = dynamic_cast<Model::EntityNode*>(linkedGroupNode->children().front());
  REQUIRE(linkedEntityNode);

  document->deselectAll();
  document->select(entityNode);

  // set the property "some_key" to protected in the original entity and change its value
  document->setProtectedProperty("some_key", true);
  document->setProperty("some_key", "some_other_value");

  linkedEntityNode = dynamic_cast<Model::EntityNode*>(linkedGroupNode->children().front());
  REQUIRE(linkedEntityNode);

  document->deselectAll();
  document->select(linkedEntityNode);

  // set the property "another_key" to protected in the linked entity and change its value
  document->setProtectedProperty("another_key", true);
  document->setProperty("another_key", "yet_another_value");

  // add another initially protected property "yet_another_key" to the linked entity
  document->setProtectedProperty("yet_another_key", true);
  document->setProperty("yet_another_key", "and_yet_another_value");

  entityNode = dynamic_cast<Model::EntityNode*>(groupNode->children().front());
  REQUIRE(entityNode);

  REQUIRE_THAT(
    entityNode->entity().protectedProperties(),
    Catch::UnorderedEquals(std::vector<std::string>{"some_key"}));
  REQUIRE_THAT(
    entityNode->entity().properties(),
    Catch::UnorderedEquals(std::vector<Model::EntityProperty>{
      {"some_key", "some_other_value"}, {"another_key", "another_value"}}));

  REQUIRE_THAT(
    linkedEntityNode->entity().protectedProperties(),
    Catch::UnorderedEquals(std::vector<std::string>{"another_key", "yet_another_key"}));
  REQUIRE_THAT(
    linkedEntityNode->entity().properties(),
    Catch::UnorderedEquals(std::vector<Model::EntityProperty>{
      {"some_key", "some_value"},
      {"another_key", "yet_another_value"},
      {"yet_another_key", "and_yet_another_value"}}));

  document->deselectAll();
  document->select(groupNode);
  document->select(linkedGroupNode);

  CHECK_FALSE(document->canClearProtectedProperties());

  document->deselect(groupNode);

  CHECK(document->canClearProtectedProperties());
  document->clearProtectedProperties();

  entityNode = dynamic_cast<Model::EntityNode*>(groupNode->children().front());
  REQUIRE(entityNode != nullptr);

  CHECK_THAT(
    entityNode->entity().protectedProperties(),
    Catch::UnorderedEquals(std::vector<std::string>{"some_key"}));
  CHECK_THAT(
    entityNode->entity().properties(), Catch::UnorderedEquals(std::vector<Model::EntityProperty>{
                                         {"some_key", "some_other_value"},
                                         {"another_key", "another_value"},
                                         {"yet_another_key", "and_yet_another_value"}}));

  CHECK_THAT(
    linkedEntityNode->entity().protectedProperties(),
    Catch::UnorderedEquals(std::vector<std::string>{}));
  CHECK_THAT(
    linkedEntityNode->entity().properties(),
    Catch::UnorderedEquals(std::vector<Model::EntityProperty>{
      {"some_key", "some_value"},
      {"another_key", "another_value"},
      {"yet_another_key", "and_yet_another_value"}}));

  document->undoCommand();

  entityNode = dynamic_cast<Model::EntityNode*>(groupNode->children().front());
  REQUIRE(entityNode != nullptr);

  CHECK_THAT(
    entityNode->entity().protectedProperties(),
    Catch::UnorderedEquals(std::vector<std::string>{"some_key"}));
  CHECK_THAT(
    entityNode->entity().properties(),
    Catch::UnorderedEquals(std::vector<Model::EntityProperty>{
      {"some_key", "some_other_value"}, {"another_key", "another_value"}}));

  CHECK_THAT(
    linkedEntityNode->entity().protectedProperties(),
    Catch::UnorderedEquals(std::vector<std::string>{"another_key", "yet_another_key"}));
  CHECK_THAT(
    linkedEntityNode->entity().properties(),
    Catch::UnorderedEquals(std::vector<Model::EntityProperty>{
      {"some_key", "some_value"},
      {"another_key", "yet_another_value"},
      {"yet_another_key", "and_yet_another_value"}}));
}

TEST_CASE_METHOD(MapDocumentTest, "EntityNodesTest.updateSpawnflagOnBrushEntity") {
  // delete default brush
  document->selectAllNodes();
  document->deleteObjects();

  const Model::BrushBuilder builder(document->world()->mapFormat(), document->worldBounds());

  auto* brushNode = new Model::BrushNode(
    builder.createCuboid(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 64)), "texture").value());
  addNode(*document, document->parentForNodes(), brushNode);

  document->selectAllNodes();

  Model::EntityNode* brushEntNode = document->createBrushEntity(m_brushEntityDef);
  REQUIRE_THAT(
    document->selectedNodes().nodes(),
    Catch::UnorderedEquals(std::vector<Model::Node*>{brushNode}));

  REQUIRE(!brushEntNode->entity().hasProperty("spawnflags"));
  CHECK(document->updateSpawnflag("spawnflags", 1, true));

  REQUIRE(brushEntNode->entity().hasProperty("spawnflags"));
  CHECK(*brushEntNode->entity().property("spawnflags") == "2");
}
} // namespace View
} // namespace TrenchBroom
