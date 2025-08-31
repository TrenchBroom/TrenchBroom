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
#include "mdl/BrushBuilder.h"
#include "mdl/BrushNode.h"
#include "mdl/Entity.h"
#include "mdl/EntityDefinition.h"
#include "mdl/EntityDefinitionManager.h"
#include "mdl/EntityNode.h"
#include "mdl/GroupNode.h"
#include "mdl/Map.h"
#include "mdl/Map_Entities.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Transaction.h"
#include "mdl/WorldNode.h"

#include "Catch2.h"

namespace tb::mdl
{

TEST_CASE("Map_Entities")
{
  auto fixture = MapFixture{};
  auto& map = fixture.map();
  fixture.create();

  map.entityDefinitionManager().setDefinitions({
    {"point_entity",
     Color{},
     "this is a point entity",
     {},
     PointEntityDefinition{vm::bbox3d{16.0}, {}, {}}},
    {
      "large_entity",
      Color{},
      "this is a point entity",
      {},
      PointEntityDefinition{vm::bbox3d{64.0}, {}, {}},
    },
    {"brush_entity", Color{}, "this is a brush entity", {}},
  });

  const auto* pointEntityDefinition =
    map.entityDefinitionManager().definition("point_entity");
  const auto* largeEntityDefinition =
    map.entityDefinitionManager().definition("large_entity");
  const auto* brushEntityDefinition =
    map.entityDefinitionManager().definition("brush_entity");

  REQUIRE(pointEntityDefinition);
  REQUIRE(largeEntityDefinition);
  REQUIRE(brushEntityDefinition);

  const auto builder = BrushBuilder{map.world()->mapFormat(), map.worldBounds()};

  SECTION("createPointEntity")
  {
    SECTION("Point entity is created and selected")
    {
      auto* entityNode =
        createPointEntity(map, *pointEntityDefinition, vm::vec3d{16.0, 32.0, 48.0});
      CHECK(entityNode != nullptr);
      CHECK(entityNode->entity().definition() == pointEntityDefinition);
      CHECK(entityNode->entity().origin() == vm::vec3d{16.0, 32.0, 48.0});
      CHECK(map.selection().nodes == std::vector<Node*>{entityNode});
    }

    SECTION("Selected objects are deselect and not translated")
    {
      auto* existingNode =
        createPointEntity(map, *pointEntityDefinition, vm::vec3d{0, 0, 0});
      map.selectNodes({existingNode});

      const auto origin = existingNode->entity().origin();
      createPointEntity(map, *pointEntityDefinition, {16, 16, 16});

      CHECK(existingNode->entity().origin() == origin);
    }

    SECTION("Default entity properties")
    {
      auto gameConfig = MockGameConfig{};
      gameConfig.entityConfig.setDefaultProperties = true;
      fixture.create({.game = MockGameFixture{std::move(gameConfig)}});

      map.entityDefinitionManager().setDefinitions({
        EntityDefinition{
          "some_name",
          Color{},
          "",
          {
            {"some_default_prop", PropertyValueTypes::String{"value"}, "", ""},
          },
          PointEntityDefinition{
            vm::bbox3d{32.0},
            {},
            {},
          },
        },
      });

      const auto& definitionWithDefaults =
        map.entityDefinitionManager().definitions().front();

      auto* entityNode = createPointEntity(map, definitionWithDefaults, {0, 0, 0});
      REQUIRE(entityNode != nullptr);
      CHECK_THAT(
        entityNode->entity().properties(),
        Catch::Matchers::UnorderedEquals(std::vector<EntityProperty>{
          {EntityPropertyKeys::Classname, "some_name"},
          {"some_default_prop", "value"},
        }));
    }

    SECTION("Linked group update failure")
    {
      auto* entityNode = new EntityNode{Entity{}};
      addNodes(map, {{parentForNodes(map), {entityNode}}});
      map.selectNodes({entityNode});

      // move the entity down
      REQUIRE(map.translateSelection({0, 0, -256}));
      REQUIRE(
        entityNode->physicalBounds() == vm::bbox3d{{-8, -8, -256 - 8}, {8, 8, -256 + 8}});

      auto* groupNode = map.groupSelectedNodes("test");
      auto* linkedGroupNode = map.createLinkedDuplicate();
      REQUIRE(linkedGroupNode);

      // move the linked group up by half the world bounds
      const auto zOffset = map.worldBounds().max.z();
      map.deselectAll();
      map.selectNodes({linkedGroupNode});
      map.translateSelection({0, 0, map.worldBounds().max.z()});
      REQUIRE(
        linkedGroupNode->physicalBounds()
        == vm::bbox3d{{-8, -8, -256 - 8 + zOffset}, {8, 8, -256 + 8 + zOffset}});

      // create a brush entity inside the original group
      map.openGroup(groupNode);
      map.deselectAll();

      // create a new point entity below the origin -- this entity is temporarily
      // created at the origin and then moved to its eventual position, but the entity
      // at the origin is propagated into the linked group, where it ends up out of
      // world bounds
      CHECK(createPointEntity(map, *pointEntityDefinition, {0, 0, -32}) != nullptr);
    }
  }

  SECTION("createBrushEntity")
  {
    SECTION("Brush entity is created and selected")
    {
      auto* brushNode = createBrushNode(map, "some_material");
      addNodes(map, {{parentForNodes(map), {brushNode}}});

      map.selectNodes({brushNode});
      auto* entityNode = createBrushEntity(map, *brushEntityDefinition);
      CHECK(entityNode != nullptr);
      CHECK(entityNode->entity().definition() == brushEntityDefinition);
      CHECK(map.selection().nodes == std::vector<Node*>{brushNode});
    }

    SECTION("Copies properties from existing brush entity")
    {
      auto* brushNode1 = createBrushNode(map, "some_material");
      auto* brushNode2 = createBrushNode(map, "some_material");
      auto* brushNode3 = createBrushNode(map, "some_material");
      addNodes(map, {{parentForNodes(map), {brushNode1, brushNode2, brushNode3}}});

      map.selectNodes({brushNode1, brushNode2, brushNode3});
      auto* previousEntityNode = createBrushEntity(map, *brushEntityDefinition);

      setEntityProperty(map, "prop", "value");
      REQUIRE(previousEntityNode->entity().hasProperty("prop", "value"));

      map.deselectAll();
      map.selectNodes({brushNode1, brushNode2});

      auto* newEntityNode = createBrushEntity(map, *brushEntityDefinition);
      CHECK(newEntityNode != nullptr);
      CHECK(newEntityNode->entity().hasProperty("prop", "value"));
    }

    SECTION("Default entity properties")
    {
      auto gameConfig = MockGameConfig{};
      gameConfig.entityConfig.setDefaultProperties = true;
      fixture.create({.game = MockGameFixture{std::move(gameConfig)}});

      map.entityDefinitionManager().setDefinitions({
        EntityDefinition{
          "some_name",
          Color{},
          "",
          {
            {"some_default_prop", PropertyValueTypes::String{"value"}, "", ""},
          },
        },
      });

      const auto& definitionWithDefaults =
        map.entityDefinitionManager().definitions().front();

      auto* brushNode = createBrushNode(map, "some_material");
      addNodes(map, {{parentForNodes(map), {brushNode}}});

      map.selectNodes({brushNode});
      auto* entityNode = createBrushEntity(map, definitionWithDefaults);
      REQUIRE(entityNode != nullptr);
      CHECK_THAT(
        entityNode->entity().properties(),
        Catch::Matchers::UnorderedEquals(std::vector<EntityProperty>{
          {EntityPropertyKeys::Classname, "some_name"},
          {"some_default_prop", "value"},
        }));
    }

    SECTION("Linked group update failure")
    {
      auto* entityNode = new EntityNode{Entity{}};
      addNodes(map, {{parentForNodes(map), {entityNode}}});
      map.selectNodes({entityNode});

      // move the entity down
      REQUIRE(map.translateSelection({0, 0, -256}));
      REQUIRE(
        entityNode->physicalBounds() == vm::bbox3d{{-8, -8, -256 - 8}, {8, 8, -256 + 8}});

      auto* groupNode = map.groupSelectedNodes("test");
      auto* linkedGroupNode = map.createLinkedDuplicate();

      // move the linked group up by half the world bounds
      const auto zOffset = map.worldBounds().max.z();
      map.deselectAll();
      map.selectNodes({linkedGroupNode});
      map.translateSelection({0, 0, map.worldBounds().max.z()});
      REQUIRE(
        linkedGroupNode->physicalBounds()
        == vm::bbox3d{{-8, -8, -256 - 8 + zOffset}, {8, 8, -256 + 8 + zOffset}});

      // create a brush entity inside the original group
      map.openGroup(groupNode);
      map.deselectAll();

      auto* brushNode = createBrushNode(map);
      transformNode(
        *brushNode, vm::translation_matrix(vm::vec3d{0, 0, -32}), map.worldBounds());
      REQUIRE(brushNode->physicalBounds() == vm::bbox3d{{-16, -16, -48}, {16, 16, -16}});

      addNodes(map, {{parentForNodes(map), {brushNode}}});
      map.deselectAll();
      map.selectNodes({brushNode});

      // create a brush entity - a temporarily empty entity will be created at the origin
      // and propagated into the linked group, where it ends up out of world bounds and
      // thus failing
      CHECK(createBrushEntity(map, *brushEntityDefinition) != nullptr);
    }
  }

  SECTION("setEntityProperty")
  {
    SECTION("Change entity class name")
    {
      auto* entityNode = new EntityNode(Entity{{{"classname", "large_entity"}}});

      addNodes(map, {{parentForNodes(map), {entityNode}}});
      REQUIRE(entityNode->entity().definition() == largeEntityDefinition);

      map.deselectAll();
      map.selectNodes({entityNode});
      REQUIRE(
        map.selectionBounds()->size()
        == largeEntityDefinition->pointEntityDefinition->bounds.size());

      setEntityProperty(map, "classname", "point_entity");
      CHECK(entityNode->entity().definition() == pointEntityDefinition);
      CHECK(
        map.selectionBounds()->size()
        == pointEntityDefinition->pointEntityDefinition->bounds.size());

      removeEntityProperty(map, "classname");
      CHECK(entityNode->entity().definition() == nullptr);
      CHECK(map.selectionBounds()->size() == EntityNode::DefaultBounds.size());
    }

    SECTION("Attempt to set a property with 2 out of 3 groups selected")
    {
      // https://github.com/TrenchBroom/TrenchBroom/issues/3768

      auto* entityNode = new EntityNode{Entity{}};
      addNodes(map, {{parentForNodes(map), {entityNode}}});
      map.selectNodes({entityNode});

      auto* groupNode = map.groupSelectedNodes("test");
      auto* linkedGroupNode1 = map.createLinkedDuplicate();
      auto* linkedGroupNode2 = map.createLinkedDuplicate();

      REQUIRE(groupNode != nullptr);
      REQUIRE(linkedGroupNode1 != nullptr);
      REQUIRE(linkedGroupNode2 != nullptr);

      map.deselectAll();

      map.selectNodes({groupNode, linkedGroupNode1});

      // Current design is to reject this because it's modifying entities from multiple
      // groups in a link set. While in this case the change isn't conflicting, some
      // entity changes are, e.g. unprotecting a property with 2 linked groups selected,
      // where entities have different values for that protected property.
      //
      // Additionally, the use case for editing entity properties with the entire map
      // selected seems unlikely.
      CHECK(!setEntityProperty(map, "key", "value"));

      auto* groupNodeEntity = dynamic_cast<EntityNode*>(groupNode->children().at(0));
      auto* linkedEntityNode1 =
        dynamic_cast<EntityNode*>(linkedGroupNode1->children().at(0));
      auto* linkedEntityNode2 =
        dynamic_cast<EntityNode*>(linkedGroupNode2->children().at(0));
      REQUIRE(groupNodeEntity != nullptr);
      REQUIRE(linkedEntityNode1 != nullptr);
      REQUIRE(linkedEntityNode2 != nullptr);

      CHECK(!groupNodeEntity->entity().hasProperty("key"));
      CHECK(!linkedEntityNode1->entity().hasProperty("key"));
      CHECK(!linkedEntityNode2->entity().hasProperty("key"));
    }
  }

  SECTION("renameEntityProperty")
  {
    SECTION("Rename entity class name")
    {
      auto* entityNode = new EntityNode(Entity{{{"classname", "large_entity"}}});

      addNodes(map, {{parentForNodes(map), {entityNode}}});
      REQUIRE(entityNode->entity().definition() == largeEntityDefinition);

      map.deselectAll();
      map.selectNodes({entityNode});
      REQUIRE(
        map.selectionBounds()->size()
        == largeEntityDefinition->pointEntityDefinition->bounds.size());

      renameEntityProperty(map, "classname", "temp");
      CHECK(entityNode->entity().definition() == nullptr);
      CHECK(map.selectionBounds()->size() == EntityNode::DefaultBounds.size());

      renameEntityProperty(map, "temp", "classname");

      CHECK(entityNode->entity().definition() == largeEntityDefinition);
      CHECK(
        map.selectionBounds()->size()
        == largeEntityDefinition->pointEntityDefinition->bounds.size());

      map.undoCommand();
      CHECK(entityNode->entity().definition() == nullptr);
      CHECK(map.selectionBounds()->size() == EntityNode::DefaultBounds.size());

      map.undoCommand();
      CHECK(entityNode->entity().definition() == largeEntityDefinition);
      CHECK(
        map.selectionBounds()->size()
        == largeEntityDefinition->pointEntityDefinition->bounds.size());
    }
  }

  SECTION("removeEntityProperty")
  {
    SECTION("Remove entity class name")
    {
      auto* entityNode = new EntityNode(Entity{{{"classname", "large_entity"}}});

      addNodes(map, {{parentForNodes(map), {entityNode}}});
      REQUIRE(entityNode->entity().definition() == largeEntityDefinition);

      map.deselectAll();
      map.selectNodes({entityNode});
      REQUIRE(
        map.selectionBounds()->size()
        == largeEntityDefinition->pointEntityDefinition->bounds.size());

      removeEntityProperty(map, "classname");
      CHECK(entityNode->entity().definition() == nullptr);
      CHECK(map.selectionBounds()->size() == EntityNode::DefaultBounds.size());

      map.undoCommand();
      CHECK(entityNode->entity().definition() == largeEntityDefinition);
      CHECK(
        map.selectionBounds()->size()
        == largeEntityDefinition->pointEntityDefinition->bounds.size());
    }
  }

  SECTION("updateEntitySpawnflag")
  {
    SECTION("Brush entity")
    {
      auto* brushNode = new BrushNode{
        builder.createCuboid(vm::bbox3d{{0, 0, 0}, {64, 64, 64}}, "material")
        | kdl::value()};
      addNodes(map, {{parentForNodes(map), {brushNode}}});

      map.selectAllNodes();

      auto* brushEntNode = createBrushEntity(map, *brushEntityDefinition);
      REQUIRE_THAT(
        map.selection().nodes, Catch::UnorderedEquals(std::vector<Node*>{brushNode}));

      REQUIRE(!brushEntNode->entity().hasProperty("spawnflags"));
      CHECK(updateEntitySpawnflag(map, "spawnflags", 1, true));

      REQUIRE(brushEntNode->entity().hasProperty("spawnflags"));
      CHECK(*brushEntNode->entity().property("spawnflags") == "2");
    }
  }

  SECTION("setProtectedEntityProperty")
  {
    SECTION("Toggle protected state")
    {
      auto* entityNode = new EntityNode{Entity{}};
      addNodes(map, {{parentForNodes(map), {entityNode}}});

      map.selectNodes({entityNode});

      SECTION("Set protected property")
      {
        setProtectedEntityProperty(map, "some_key", true);
        CHECK_THAT(
          entityNode->entity().protectedProperties(),
          Catch::UnorderedEquals(std::vector<std::string>{"some_key"}));

        map.undoCommand();
        CHECK_THAT(
          entityNode->entity().protectedProperties(),
          Catch::UnorderedEquals(std::vector<std::string>{}));
      }

      SECTION("Unset protected property")
      {
        setProtectedEntityProperty(map, "some_key", true);
        REQUIRE_THAT(
          entityNode->entity().protectedProperties(),
          Catch::UnorderedEquals(std::vector<std::string>{"some_key"}));

        // Ensure that the consecutive SwapNodeContentsCommands are not collated
        map.deselectAll();
        map.selectNodes({entityNode});

        setProtectedEntityProperty(map, "some_key", false);
        CHECK_THAT(
          entityNode->entity().protectedProperties(),
          Catch::UnorderedEquals(std::vector<std::string>{}));

        map.undoCommand();
        CHECK_THAT(
          entityNode->entity().protectedProperties(),
          Catch::UnorderedEquals(std::vector<std::string>{"some_key"}));
      }
    }

    SECTION("Setting protected entity properties restores their values")
    {
      auto* entityNode = new EntityNode{Entity{{{"some_key", "some_value"}}}};
      addNodes(map, {{parentForNodes(map), {entityNode}}});

      map.selectNodes({entityNode});
      auto* groupNode = map.groupSelectedNodes("test");

      map.deselectAll();
      map.selectNodes({groupNode});

      auto* linkedGroupNode = map.createLinkedDuplicate();
      REQUIRE(linkedGroupNode->childCount() == 1u);

      // both entities have the same value initially
      auto* linkedEntityNode =
        dynamic_cast<EntityNode*>(linkedGroupNode->children().front());
      REQUIRE(linkedEntityNode);
      REQUIRE_THAT(
        linkedEntityNode->entity().properties(),
        Catch::UnorderedEquals(std::vector<EntityProperty>{{"some_key", "some_value"}}));

      map.deselectAll();
      map.selectNodes({linkedEntityNode});

      // set the property to protected in the linked entity and change its value
      setProtectedEntityProperty(map, "some_key", true);
      setEntityProperty(map, "some_key", "another_value");
      REQUIRE_THAT(
        linkedEntityNode->entity().properties(),
        Catch::UnorderedEquals(
          std::vector<EntityProperty>{{"some_key", "another_value"}}));

      // the value in the original entity remains unchanged
      entityNode = dynamic_cast<EntityNode*>(groupNode->children().front());
      REQUIRE_THAT(
        entityNode->entity().properties(),
        Catch::UnorderedEquals(std::vector<EntityProperty>{{"some_key", "some_value"}}));

      SECTION("When there is an unprotected property in the corresponding entity")
      {
        // set the property to unprotected, now the original value should be restored
        setProtectedEntityProperty(map, "some_key", false);

        entityNode = dynamic_cast<EntityNode*>(groupNode->children().front());
        CHECK_THAT(
          linkedEntityNode->entity().properties(),
          Catch::UnorderedEquals(
            std::vector<EntityProperty>{{"some_key", "some_value"}}));
        CHECK_THAT(
          entityNode->entity().properties(),
          Catch::UnorderedEquals(
            std::vector<EntityProperty>{{"some_key", "some_value"}}));
      }

      SECTION("When no corresponding entity with an unprotected property can be found")
      {
        // set the property to protected in the original entity too
        map.deselectAll();
        map.selectNodes({entityNode});
        setProtectedEntityProperty(map, "some_key", true);

        linkedEntityNode = dynamic_cast<EntityNode*>(linkedGroupNode->children().front());
        REQUIRE_THAT(
          entityNode->entity().properties(),
          Catch::UnorderedEquals(
            std::vector<EntityProperty>{{"some_key", "some_value"}}));
        REQUIRE_THAT(
          linkedEntityNode->entity().properties(),
          Catch::UnorderedEquals(
            std::vector<EntityProperty>{{"some_key", "another_value"}}));

        map.deselectAll();
        map.selectNodes({linkedEntityNode});
        setProtectedEntityProperty(map, "some_key", false);

        entityNode = dynamic_cast<EntityNode*>(groupNode->children().front());
        CHECK_THAT(
          entityNode->entity().properties(),
          Catch::UnorderedEquals(
            std::vector<EntityProperty>{{"some_key", "some_value"}}));
        CHECK_THAT(
          linkedEntityNode->entity().properties(),
          Catch::UnorderedEquals(
            std::vector<EntityProperty>{{"some_key", "another_value"}}));

        SECTION(
          "Setting the property to unprotected in the original entity will fetch the new "
          "value now")
        {
          map.deselectAll();
          map.selectNodes({entityNode});
          setProtectedEntityProperty(map, "some_key", false);

          linkedEntityNode =
            dynamic_cast<EntityNode*>(linkedGroupNode->children().front());
          CHECK_THAT(
            entityNode->entity().properties(),
            Catch::UnorderedEquals(
              std::vector<EntityProperty>{{"some_key", "another_value"}}));
          CHECK_THAT(
            linkedEntityNode->entity().properties(),
            Catch::UnorderedEquals(
              std::vector<EntityProperty>{{"some_key", "another_value"}}));
        }
      }

      SECTION("When setting a property to unprotected that only exists in one entity")
      {
        setProtectedEntityProperty(map, "yet_another_key", true);
        setEntityProperty(map, "yet_another_key", "yet_another_value");

        entityNode = dynamic_cast<EntityNode*>(groupNode->children().front());
        REQUIRE_THAT(
          entityNode->entity().properties(),
          Catch::UnorderedEquals(
            std::vector<EntityProperty>{{"some_key", "some_value"}}));
        REQUIRE_THAT(
          linkedEntityNode->entity().properties(),
          Catch::UnorderedEquals(std::vector<EntityProperty>{
            {"some_key", "another_value"},
            {"yet_another_key", "yet_another_value"},
          }));

        setProtectedEntityProperty(map, "yet_another_key", false);

        entityNode = dynamic_cast<EntityNode*>(groupNode->children().front());
        CHECK_THAT(
          entityNode->entity().properties(),
          Catch::UnorderedEquals(std::vector<EntityProperty>{
            {"some_key", "some_value"},
            {"yet_another_key", "yet_another_value"},
          }));
        CHECK_THAT(
          linkedEntityNode->entity().properties(),
          Catch::UnorderedEquals(std::vector<EntityProperty>{
            {"some_key", "another_value"},
            {"yet_another_key", "yet_another_value"},
          }));
      }
    }
  }

  SECTION("clearProtectedEntityProperties")
  {
    auto* entityNode = new EntityNode{Entity{{
      {"some_key", "some_value"},
      {"another_key", "another_value"},
    }}};
    addNodes(map, {{parentForNodes(map), {entityNode}}});

    CHECK_FALSE(canClearProtectedEntityProperties(map));

    map.selectNodes({entityNode});
    CHECK(canClearProtectedEntityProperties(map));

    auto* groupNode = map.groupSelectedNodes("test");

    map.deselectAll();
    map.selectNodes({groupNode});
    CHECK(canClearProtectedEntityProperties(map));

    auto* linkedGroupNode = map.createLinkedDuplicate();
    REQUIRE(linkedGroupNode->childCount() == 1u);

    // both entities have the same values initially
    auto* linkedEntityNode =
      dynamic_cast<EntityNode*>(linkedGroupNode->children().front());
    REQUIRE(linkedEntityNode);

    map.deselectAll();
    map.selectNodes({entityNode});

    // set the property "some_key" to protected in the original entity and change its
    // value
    setProtectedEntityProperty(map, "some_key", true);
    setEntityProperty(map, "some_key", "some_other_value");

    linkedEntityNode = dynamic_cast<EntityNode*>(linkedGroupNode->children().front());
    REQUIRE(linkedEntityNode);

    map.deselectAll();
    map.selectNodes({linkedEntityNode});

    // set the property "another_key" to protected in the linked entity and change its
    // value
    setProtectedEntityProperty(map, "another_key", true);
    setEntityProperty(map, "another_key", "yet_another_value");

    // add another initially protected property "yet_another_key" to the linked entity
    setProtectedEntityProperty(map, "yet_another_key", true);
    setEntityProperty(map, "yet_another_key", "and_yet_another_value");

    entityNode = dynamic_cast<EntityNode*>(groupNode->children().front());
    REQUIRE(entityNode);

    REQUIRE_THAT(
      entityNode->entity().protectedProperties(),
      Catch::UnorderedEquals(std::vector<std::string>{"some_key"}));
    REQUIRE_THAT(
      entityNode->entity().properties(),
      Catch::UnorderedEquals(std::vector<EntityProperty>{
        {"some_key", "some_other_value"}, {"another_key", "another_value"}}));

    REQUIRE_THAT(
      linkedEntityNode->entity().protectedProperties(),
      Catch::UnorderedEquals(std::vector<std::string>{"another_key", "yet_another_key"}));
    REQUIRE_THAT(
      linkedEntityNode->entity().properties(),
      Catch::UnorderedEquals(std::vector<EntityProperty>{
        {"some_key", "some_value"},
        {"another_key", "yet_another_value"},
        {"yet_another_key", "and_yet_another_value"}}));

    map.deselectAll();
    map.selectNodes({groupNode});
    map.selectNodes({linkedGroupNode});

    CHECK_FALSE(canClearProtectedEntityProperties(map));

    map.deselectNodes({groupNode});

    CHECK(canClearProtectedEntityProperties(map));
    clearProtectedEntityProperties(map);

    entityNode = dynamic_cast<EntityNode*>(groupNode->children().front());
    REQUIRE(entityNode != nullptr);

    CHECK_THAT(
      entityNode->entity().protectedProperties(),
      Catch::UnorderedEquals(std::vector<std::string>{"some_key"}));
    CHECK_THAT(
      entityNode->entity().properties(),
      Catch::UnorderedEquals(std::vector<EntityProperty>{
        {"some_key", "some_other_value"},
        {"another_key", "another_value"},
        {"yet_another_key", "and_yet_another_value"}}));

    CHECK_THAT(
      linkedEntityNode->entity().protectedProperties(),
      Catch::UnorderedEquals(std::vector<std::string>{}));
    CHECK_THAT(
      linkedEntityNode->entity().properties(),
      Catch::UnorderedEquals(std::vector<EntityProperty>{
        {"some_key", "some_value"},
        {"another_key", "another_value"},
        {"yet_another_key", "and_yet_another_value"}}));

    map.undoCommand();

    entityNode = dynamic_cast<EntityNode*>(groupNode->children().front());
    REQUIRE(entityNode != nullptr);

    CHECK_THAT(
      entityNode->entity().protectedProperties(),
      Catch::UnorderedEquals(std::vector<std::string>{"some_key"}));
    CHECK_THAT(
      entityNode->entity().properties(),
      Catch::UnorderedEquals(std::vector<EntityProperty>{
        {"some_key", "some_other_value"}, {"another_key", "another_value"}}));

    CHECK_THAT(
      linkedEntityNode->entity().protectedProperties(),
      Catch::UnorderedEquals(std::vector<std::string>{"another_key", "yet_another_key"}));
    CHECK_THAT(
      linkedEntityNode->entity().properties(),
      Catch::UnorderedEquals(std::vector<EntityProperty>{
        {"some_key", "some_value"},
        {"another_key", "yet_another_value"},
        {"yet_another_key", "and_yet_another_value"}}));
  }

  SECTION("setDefaultEntityProperties")
  {
    map.entityDefinitionManager().setDefinitions({
      EntityDefinition{
        "some_name",
        Color{},
        "",
        {
          {"some_prop", PropertyValueTypes::String{}, "", ""},
          {"default_prop_a", PropertyValueTypes::String{"default_value_a"}, "", ""},
          {"default_prop_b", PropertyValueTypes::String{"default_value_b"}, "", ""},
        },
        PointEntityDefinition{
          vm::bbox3d{32.0},
          {},
          {},
        },
      },
    });

    const auto* definitionWithDefaults =
      map.entityDefinitionManager().definition("some_name");
    REQUIRE(definitionWithDefaults);

    auto* entityNodeWithoutDefinition = new EntityNode{Entity{{
      {"classname", "some_class"},
    }}};
    addNodes(map, {{parentForNodes(map), {entityNodeWithoutDefinition}}});
    map.selectNodes({entityNodeWithoutDefinition});
    setEntityProperty(map, "some_prop", "some_value");
    map.deselectAll();

    auto* entityNodeWithProp = createPointEntity(map, *definitionWithDefaults, {0, 0, 0});
    REQUIRE(entityNodeWithProp != nullptr);
    REQUIRE(entityNodeWithProp->entity().definition() == definitionWithDefaults);
    map.selectNodes({entityNodeWithProp});
    setEntityProperty(map, "some_prop", "some_value");
    map.deselectAll();

    auto* entityNodeWithPropA =
      createPointEntity(map, *definitionWithDefaults, {0, 0, 0});
    REQUIRE(entityNodeWithPropA != nullptr);
    REQUIRE(entityNodeWithPropA->entity().definition() == definitionWithDefaults);
    map.selectNodes({entityNodeWithPropA});
    setEntityProperty(map, "some_prop", "some_value");
    setEntityProperty(map, "default_prop_a", "default_value_a");
    map.deselectAll();

    auto* entityNodeWithPropAWithValueChanged =
      createPointEntity(map, *definitionWithDefaults, {0, 0, 0});
    REQUIRE(entityNodeWithPropAWithValueChanged != nullptr);
    REQUIRE(
      entityNodeWithPropAWithValueChanged->entity().definition()
      == definitionWithDefaults);
    map.selectNodes({entityNodeWithPropAWithValueChanged});
    setEntityProperty(map, "default_prop_a", "some_other_value");
    map.deselectAll();

    auto* entityNodeWithPropsAB =
      createPointEntity(map, *definitionWithDefaults, {0, 0, 0});
    REQUIRE(entityNodeWithPropsAB != nullptr);
    REQUIRE(entityNodeWithPropsAB->entity().definition() == definitionWithDefaults);
    map.selectNodes({entityNodeWithPropsAB});
    setEntityProperty(map, "some_prop", "some_value");
    setEntityProperty(map, "default_prop_a", "default_value_a");
    setEntityProperty(map, "default_prop_b", "yet_another_value");
    map.deselectAll();

    REQUIRE_THAT(
      entityNodeWithoutDefinition->entity().properties(),
      Catch::Matchers::UnorderedEquals(std::vector<EntityProperty>{
        {"classname", "some_class"},
        {"some_prop", "some_value"},
      }));
    REQUIRE_THAT(
      entityNodeWithProp->entity().properties(),
      Catch::Matchers::UnorderedEquals(std::vector<EntityProperty>{
        {"classname", "some_name"},
        {"some_prop", "some_value"},
      }));
    REQUIRE_THAT(
      entityNodeWithPropA->entity().properties(),
      Catch::Matchers::UnorderedEquals(std::vector<EntityProperty>{
        {"classname", "some_name"},
        {"some_prop", "some_value"},
        {"default_prop_a", "default_value_a"},
      }));
    REQUIRE_THAT(
      entityNodeWithPropAWithValueChanged->entity().properties(),
      Catch::Matchers::UnorderedEquals(std::vector<EntityProperty>{
        {"classname", "some_name"},
        {"default_prop_a", "some_other_value"},
      }));
    REQUIRE_THAT(
      entityNodeWithPropsAB->entity().properties(),
      Catch::Matchers::UnorderedEquals(std::vector<EntityProperty>{
        {"classname", "some_name"},
        {"some_prop", "some_value"},
        {"default_prop_a", "default_value_a"},
        {"default_prop_b", "yet_another_value"},
      }));

    map.selectNodes(
      {entityNodeWithoutDefinition,
       entityNodeWithProp,
       entityNodeWithPropA,
       entityNodeWithPropAWithValueChanged,
       entityNodeWithPropsAB});

    SECTION("Set Existing Default Properties")
    {
      setDefaultEntityProperties(map, SetDefaultPropertyMode::SetExisting);

      CHECK_THAT(
        entityNodeWithoutDefinition->entity().properties(),
        Catch::Matchers::UnorderedEquals(std::vector<EntityProperty>{
          {"classname", "some_class"},
          {"some_prop", "some_value"},
        }));
      CHECK_THAT(
        entityNodeWithProp->entity().properties(),
        Catch::Matchers::UnorderedEquals(std::vector<EntityProperty>{
          {"classname", "some_name"},
          {"some_prop", "some_value"},
        }));
      CHECK_THAT(
        entityNodeWithPropA->entity().properties(),
        Catch::Matchers::UnorderedEquals(std::vector<EntityProperty>{
          {"classname", "some_name"},
          {"some_prop", "some_value"},
          {"default_prop_a", "default_value_a"},
        }));
      CHECK_THAT(
        entityNodeWithPropAWithValueChanged->entity().properties(),
        Catch::Matchers::UnorderedEquals(std::vector<EntityProperty>{
          {"classname", "some_name"},
          {"default_prop_a", "default_value_a"},
        }));
      CHECK_THAT(
        entityNodeWithPropsAB->entity().properties(),
        Catch::Matchers::UnorderedEquals(std::vector<EntityProperty>{
          {"classname", "some_name"},
          {"some_prop", "some_value"},
          {"default_prop_a", "default_value_a"},
          {"default_prop_b", "default_value_b"},
        }));
    }

    SECTION("Set Missing Default Properties")
    {
      setDefaultEntityProperties(map, SetDefaultPropertyMode::SetMissing);

      CHECK_THAT(
        entityNodeWithoutDefinition->entity().properties(),
        Catch::Matchers::UnorderedEquals(std::vector<EntityProperty>{
          {"classname", "some_class"},
          {"some_prop", "some_value"},
        }));
      CHECK_THAT(
        entityNodeWithProp->entity().properties(),
        Catch::Matchers::UnorderedEquals(std::vector<EntityProperty>{
          {"classname", "some_name"},
          {"some_prop", "some_value"},
          {"default_prop_a", "default_value_a"},
          {"default_prop_b", "default_value_b"},
        }));
      CHECK_THAT(
        entityNodeWithPropA->entity().properties(),
        Catch::Matchers::UnorderedEquals(std::vector<EntityProperty>{
          {"classname", "some_name"},
          {"some_prop", "some_value"},
          {"default_prop_a", "default_value_a"},
          {"default_prop_b", "default_value_b"},
        }));
      CHECK_THAT(
        entityNodeWithPropAWithValueChanged->entity().properties(),
        Catch::Matchers::UnorderedEquals(std::vector<EntityProperty>{
          {"classname", "some_name"},
          {"default_prop_a", "some_other_value"},
          {"default_prop_b", "default_value_b"},
        }));
      CHECK_THAT(
        entityNodeWithPropsAB->entity().properties(),
        Catch::Matchers::UnorderedEquals(std::vector<EntityProperty>{
          {"classname", "some_name"},
          {"some_prop", "some_value"},
          {"default_prop_a", "default_value_a"},
          {"default_prop_b", "yet_another_value"},
        }));
    }

    SECTION("Set All Default Properties")
    {
      setDefaultEntityProperties(map, SetDefaultPropertyMode::SetAll);

      CHECK_THAT(
        entityNodeWithoutDefinition->entity().properties(),
        Catch::Matchers::UnorderedEquals(std::vector<EntityProperty>{
          {"classname", "some_class"},
          {"some_prop", "some_value"},
        }));
      CHECK_THAT(
        entityNodeWithProp->entity().properties(),
        Catch::Matchers::UnorderedEquals(std::vector<EntityProperty>{
          {"classname", "some_name"},
          {"some_prop", "some_value"},
          {"default_prop_a", "default_value_a"},
          {"default_prop_b", "default_value_b"},
        }));
      CHECK_THAT(
        entityNodeWithPropA->entity().properties(),
        Catch::Matchers::UnorderedEquals(std::vector<EntityProperty>{
          {"classname", "some_name"},
          {"some_prop", "some_value"},
          {"default_prop_a", "default_value_a"},
          {"default_prop_b", "default_value_b"},
        }));
      CHECK_THAT(
        entityNodeWithPropAWithValueChanged->entity().properties(),
        Catch::Matchers::UnorderedEquals(std::vector<EntityProperty>{
          {"classname", "some_name"},
          {"default_prop_a", "default_value_a"},
          {"default_prop_b", "default_value_b"},
        }));
      CHECK_THAT(
        entityNodeWithPropsAB->entity().properties(),
        Catch::Matchers::UnorderedEquals(std::vector<EntityProperty>{
          {"classname", "some_name"},
          {"some_prop", "some_value"},
          {"default_prop_a", "default_value_a"},
          {"default_prop_b", "default_value_b"},
        }));
    }
  }
}

} // namespace tb::mdl
