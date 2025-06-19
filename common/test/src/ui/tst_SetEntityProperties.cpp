/*
 Copyright (C) 2010 Kristian Duske

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

#include "Color.h"
#include "TestUtils.h"
#include "mdl/BrushBuilder.h"
#include "mdl/BrushNode.h"
#include "mdl/Entity.h"
#include "mdl/EntityDefinition.h"
#include "mdl/EntityDefinitionManager.h"
#include "mdl/EntityNode.h"
#include "mdl/GroupNode.h"
#include "mdl/Transaction.h"
#include "mdl/WorldNode.h"
#include "ui/MapDocument.h"
#include "ui/MapDocumentTest.h"

#include "kdl/result.h"

#include <vector>

#include "Catch2.h"

namespace tb::ui
{

TEST_CASE_METHOD(ValveMapDocumentTest, "SetEntityPropertiesTest.changeClassname")
{
  // need to recreate these because document->setEntityDefinitions will delete the old
  // ones
  document->setEntityDefinitions({
    {
      "point_entity",
      Color{},
      "this is a point entity",
      {},
      mdl::PointEntityDefinition{vm::bbox3d{16.0}, {}, {}},
    },
    {
      "large_entity",
      Color{},
      "this is a point entity",
      {},
      mdl::PointEntityDefinition{vm::bbox3d{64.0}, {}, {}},
    },
  });

  m_pointEntityDef = &document->entityDefinitionManager().definitions()[0];
  auto* largeEntityDef = &document->entityDefinitionManager().definitions()[1];

  auto* entityNode = new mdl::EntityNode(mdl::Entity{{{"classname", "large_entity"}}});

  document->addNodes({{document->parentForNodes(), {entityNode}}});
  REQUIRE(entityNode->entity().definition() == largeEntityDef);

  document->deselectAll();
  document->selectNodes({entityNode});
  REQUIRE(
    document->selectionBounds()->size()
    == largeEntityDef->pointEntityDefinition->bounds.size());

  document->setProperty("classname", "point_entity");
  CHECK(entityNode->entity().definition() == m_pointEntityDef);
  CHECK(
    document->selectionBounds()->size()
    == m_pointEntityDef->pointEntityDefinition->bounds.size());

  document->removeProperty("classname");
  CHECK(entityNode->entity().definition() == nullptr);
  CHECK(document->selectionBounds()->size() == mdl::EntityNode::DefaultBounds.size());

  {
    // we only want to undo the following changes later
    auto transaction = Transaction{document};
    document->setProperty("temp", "large_entity");
    document->renameProperty("temp", "classname");
    transaction.commit();

    CHECK(entityNode->entity().definition() == largeEntityDef);
    CHECK(
      document->selectionBounds()->size()
      == largeEntityDef->pointEntityDefinition->bounds.size());
  }

  document->undoCommand();
  CHECK(entityNode->entity().definition() == nullptr);
  CHECK(document->selectionBounds()->size() == mdl::EntityNode::DefaultBounds.size());
}

TEST_CASE_METHOD(ValveMapDocumentTest, "SetEntityPropertiesTest.setProtectedProperty")
{
  auto* entityNode = new mdl::EntityNode{mdl::Entity{}};
  document->addNodes({{document->parentForNodes(), {entityNode}}});

  document->selectNodes({entityNode});

  SECTION("Set protected property")
  {
    document->setProtectedProperty("some_key", true);
    CHECK_THAT(
      entityNode->entity().protectedProperties(),
      Catch::UnorderedEquals(std::vector<std::string>{"some_key"}));

    document->undoCommand();
    CHECK_THAT(
      entityNode->entity().protectedProperties(),
      Catch::UnorderedEquals(std::vector<std::string>{}));
  }

  SECTION("Unset protected property")
  {
    document->setProtectedProperty("some_key", true);
    REQUIRE_THAT(
      entityNode->entity().protectedProperties(),
      Catch::UnorderedEquals(std::vector<std::string>{"some_key"}));

    // Ensure that the consecutive SwapNodeContentsCommands are not collated
    document->deselectAll();
    document->selectNodes({entityNode});

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
  ValveMapDocumentTest, "SetEntityPropertiesTest.setProtectedPropertyRestoresValue")
{
  auto* entityNode = new mdl::EntityNode{mdl::Entity{{{"some_key", "some_value"}}}};
  document->addNodes({{document->parentForNodes(), {entityNode}}});

  document->selectNodes({entityNode});
  auto* groupNode = document->groupSelection("test");

  document->deselectAll();
  document->selectNodes({groupNode});

  auto* linkedGroupNode = document->createLinkedDuplicate();
  REQUIRE(linkedGroupNode->childCount() == 1u);

  // both entities have the same value initially
  auto* linkedEntityNode =
    dynamic_cast<mdl::EntityNode*>(linkedGroupNode->children().front());
  REQUIRE(linkedEntityNode);
  REQUIRE_THAT(
    linkedEntityNode->entity().properties(),
    Catch::UnorderedEquals(std::vector<mdl::EntityProperty>{{"some_key", "some_value"}}));

  document->deselectAll();
  document->selectNodes({linkedEntityNode});

  // set the property to protected in the linked entity and change its value
  document->setProtectedProperty("some_key", true);
  document->setProperty("some_key", "another_value");
  REQUIRE_THAT(
    linkedEntityNode->entity().properties(),
    Catch::UnorderedEquals(
      std::vector<mdl::EntityProperty>{{"some_key", "another_value"}}));

  // the value in the original entity remains unchanged
  entityNode = dynamic_cast<mdl::EntityNode*>(groupNode->children().front());
  REQUIRE_THAT(
    entityNode->entity().properties(),
    Catch::UnorderedEquals(std::vector<mdl::EntityProperty>{{"some_key", "some_value"}}));

  SECTION("When there is an unprotected property in the corresponding entity")
  {
    // set the property to unprotected, now the original value should be restored
    document->setProtectedProperty("some_key", false);

    entityNode = dynamic_cast<mdl::EntityNode*>(groupNode->children().front());
    CHECK_THAT(
      linkedEntityNode->entity().properties(),
      Catch::UnorderedEquals(
        std::vector<mdl::EntityProperty>{{"some_key", "some_value"}}));
    CHECK_THAT(
      entityNode->entity().properties(),
      Catch::UnorderedEquals(
        std::vector<mdl::EntityProperty>{{"some_key", "some_value"}}));
  }

  SECTION("When no corresponding entity with an unprotected property can be found")
  {
    // set the property to protected in the original entity too
    document->deselectAll();
    document->selectNodes({entityNode});
    document->setProtectedProperty("some_key", true);

    linkedEntityNode =
      dynamic_cast<mdl::EntityNode*>(linkedGroupNode->children().front());
    REQUIRE_THAT(
      entityNode->entity().properties(),
      Catch::UnorderedEquals(
        std::vector<mdl::EntityProperty>{{"some_key", "some_value"}}));
    REQUIRE_THAT(
      linkedEntityNode->entity().properties(),
      Catch::UnorderedEquals(
        std::vector<mdl::EntityProperty>{{"some_key", "another_value"}}));

    document->deselectAll();
    document->selectNodes({linkedEntityNode});
    document->setProtectedProperty("some_key", false);

    entityNode = dynamic_cast<mdl::EntityNode*>(groupNode->children().front());
    CHECK_THAT(
      entityNode->entity().properties(),
      Catch::UnorderedEquals(
        std::vector<mdl::EntityProperty>{{"some_key", "some_value"}}));
    CHECK_THAT(
      linkedEntityNode->entity().properties(),
      Catch::UnorderedEquals(
        std::vector<mdl::EntityProperty>{{"some_key", "another_value"}}));

    SECTION(
      "Setting the property to unprotected in the original entity will fetch the new "
      "value now")
    {
      document->deselectAll();
      document->selectNodes({entityNode});
      document->setProtectedProperty("some_key", false);

      linkedEntityNode =
        dynamic_cast<mdl::EntityNode*>(linkedGroupNode->children().front());
      CHECK_THAT(
        entityNode->entity().properties(),
        Catch::UnorderedEquals(
          std::vector<mdl::EntityProperty>{{"some_key", "another_value"}}));
      CHECK_THAT(
        linkedEntityNode->entity().properties(),
        Catch::UnorderedEquals(
          std::vector<mdl::EntityProperty>{{"some_key", "another_value"}}));
    }
  }

  SECTION("When setting a property to unprotected that only exists in one entity")
  {
    document->setProtectedProperty("yet_another_key", true);
    document->setProperty("yet_another_key", "yet_another_value");

    entityNode = dynamic_cast<mdl::EntityNode*>(groupNode->children().front());
    REQUIRE_THAT(
      entityNode->entity().properties(),
      Catch::UnorderedEquals(
        std::vector<mdl::EntityProperty>{{"some_key", "some_value"}}));
    REQUIRE_THAT(
      linkedEntityNode->entity().properties(),
      Catch::UnorderedEquals(std::vector<mdl::EntityProperty>{
        {"some_key", "another_value"},
        {"yet_another_key", "yet_another_value"},
      }));

    document->setProtectedProperty("yet_another_key", false);

    entityNode = dynamic_cast<mdl::EntityNode*>(groupNode->children().front());
    CHECK_THAT(
      entityNode->entity().properties(),
      Catch::UnorderedEquals(std::vector<mdl::EntityProperty>{
        {"some_key", "some_value"},
        {"yet_another_key", "yet_another_value"},
      }));
    CHECK_THAT(
      linkedEntityNode->entity().properties(),
      Catch::UnorderedEquals(std::vector<mdl::EntityProperty>{
        {"some_key", "another_value"},
        {"yet_another_key", "yet_another_value"},
      }));
  }
}

TEST_CASE_METHOD(ValveMapDocumentTest, "SetEntityPropertiesTest.clearProtectedProperties")
{
  auto* entityNode = new mdl::EntityNode{mdl::Entity{{
    {"some_key", "some_value"},
    {"another_key", "another_value"},
  }}};
  document->addNodes({{document->parentForNodes(), {entityNode}}});

  CHECK_FALSE(document->canClearProtectedProperties());

  document->selectNodes({entityNode});
  CHECK(document->canClearProtectedProperties());

  auto* groupNode = document->groupSelection("test");

  document->deselectAll();
  document->selectNodes({groupNode});
  CHECK(document->canClearProtectedProperties());

  auto* linkedGroupNode = document->createLinkedDuplicate();
  REQUIRE(linkedGroupNode->childCount() == 1u);

  // both entities have the same values initially
  auto* linkedEntityNode =
    dynamic_cast<mdl::EntityNode*>(linkedGroupNode->children().front());
  REQUIRE(linkedEntityNode);

  document->deselectAll();
  document->selectNodes({entityNode});

  // set the property "some_key" to protected in the original entity and change its value
  document->setProtectedProperty("some_key", true);
  document->setProperty("some_key", "some_other_value");

  linkedEntityNode = dynamic_cast<mdl::EntityNode*>(linkedGroupNode->children().front());
  REQUIRE(linkedEntityNode);

  document->deselectAll();
  document->selectNodes({linkedEntityNode});

  // set the property "another_key" to protected in the linked entity and change its value
  document->setProtectedProperty("another_key", true);
  document->setProperty("another_key", "yet_another_value");

  // add another initially protected property "yet_another_key" to the linked entity
  document->setProtectedProperty("yet_another_key", true);
  document->setProperty("yet_another_key", "and_yet_another_value");

  entityNode = dynamic_cast<mdl::EntityNode*>(groupNode->children().front());
  REQUIRE(entityNode);

  REQUIRE_THAT(
    entityNode->entity().protectedProperties(),
    Catch::UnorderedEquals(std::vector<std::string>{"some_key"}));
  REQUIRE_THAT(
    entityNode->entity().properties(),
    Catch::UnorderedEquals(std::vector<mdl::EntityProperty>{
      {"some_key", "some_other_value"}, {"another_key", "another_value"}}));

  REQUIRE_THAT(
    linkedEntityNode->entity().protectedProperties(),
    Catch::UnorderedEquals(std::vector<std::string>{"another_key", "yet_another_key"}));
  REQUIRE_THAT(
    linkedEntityNode->entity().properties(),
    Catch::UnorderedEquals(std::vector<mdl::EntityProperty>{
      {"some_key", "some_value"},
      {"another_key", "yet_another_value"},
      {"yet_another_key", "and_yet_another_value"}}));

  document->deselectAll();
  document->selectNodes({groupNode});
  document->selectNodes({linkedGroupNode});

  CHECK_FALSE(document->canClearProtectedProperties());

  document->deselectNodes({groupNode});

  CHECK(document->canClearProtectedProperties());
  document->clearProtectedProperties();

  entityNode = dynamic_cast<mdl::EntityNode*>(groupNode->children().front());
  REQUIRE(entityNode != nullptr);

  CHECK_THAT(
    entityNode->entity().protectedProperties(),
    Catch::UnorderedEquals(std::vector<std::string>{"some_key"}));
  CHECK_THAT(
    entityNode->entity().properties(),
    Catch::UnorderedEquals(std::vector<mdl::EntityProperty>{
      {"some_key", "some_other_value"},
      {"another_key", "another_value"},
      {"yet_another_key", "and_yet_another_value"}}));

  CHECK_THAT(
    linkedEntityNode->entity().protectedProperties(),
    Catch::UnorderedEquals(std::vector<std::string>{}));
  CHECK_THAT(
    linkedEntityNode->entity().properties(),
    Catch::UnorderedEquals(std::vector<mdl::EntityProperty>{
      {"some_key", "some_value"},
      {"another_key", "another_value"},
      {"yet_another_key", "and_yet_another_value"}}));

  document->undoCommand();

  entityNode = dynamic_cast<mdl::EntityNode*>(groupNode->children().front());
  REQUIRE(entityNode != nullptr);

  CHECK_THAT(
    entityNode->entity().protectedProperties(),
    Catch::UnorderedEquals(std::vector<std::string>{"some_key"}));
  CHECK_THAT(
    entityNode->entity().properties(),
    Catch::UnorderedEquals(std::vector<mdl::EntityProperty>{
      {"some_key", "some_other_value"}, {"another_key", "another_value"}}));

  CHECK_THAT(
    linkedEntityNode->entity().protectedProperties(),
    Catch::UnorderedEquals(std::vector<std::string>{"another_key", "yet_another_key"}));
  CHECK_THAT(
    linkedEntityNode->entity().properties(),
    Catch::UnorderedEquals(std::vector<mdl::EntityProperty>{
      {"some_key", "some_value"},
      {"another_key", "yet_another_value"},
      {"yet_another_key", "and_yet_another_value"}}));
}

TEST_CASE_METHOD(MapDocumentTest, "EntityNodesTest.updateSpawnflagOnBrushEntity")
{
  // delete default brush
  document->selectAllNodes();
  document->remove();

  const auto builder =
    mdl::BrushBuilder{document->world()->mapFormat(), document->worldBounds()};

  auto* brushNode = new mdl::BrushNode{
    builder.createCuboid(vm::bbox3d{{0, 0, 0}, {64, 64, 64}}, "material") | kdl::value()};
  document->addNodes({{document->parentForNodes(), {brushNode}}});

  document->selectAllNodes();

  auto* brushEntNode = document->createBrushEntity(*m_brushEntityDef);
  REQUIRE_THAT(
    document->selection().nodes,
    Catch::UnorderedEquals(std::vector<mdl::Node*>{brushNode}));

  REQUIRE(!brushEntNode->entity().hasProperty("spawnflags"));
  CHECK(document->updateSpawnflag("spawnflags", 1, true));

  REQUIRE(brushEntNode->entity().hasProperty("spawnflags"));
  CHECK(*brushEntNode->entity().property("spawnflags") == "2");
}

} // namespace tb::ui
