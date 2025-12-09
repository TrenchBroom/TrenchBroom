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

#include "TestFactory.h"
#include "TestUtils.h"
#include "mdl/BrushBuilder.h"
#include "mdl/BrushNode.h"
#include "mdl/Entity.h"
#include "mdl/EntityDefinition.h"
#include "mdl/EntityDefinitionManager.h"
#include "mdl/EntityNode.h"
#include "mdl/GroupNode.h"
#include "mdl/LayerNode.h"
#include "mdl/Map.h"
#include "mdl/MapFixture.h"
#include "mdl/Map_Entities.h"
#include "mdl/Map_Geometry.h"
#include "mdl/Map_Groups.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "mdl/Transaction.h"
#include "mdl/WorldNode.h"

#include "catch/CatchConfig.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

namespace tb::mdl
{
using namespace Catch::Matchers;

TEST_CASE("Map_Entities")
{
  auto fixture = MapFixture{};
  auto& map = fixture.create();

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
    {
      "color_entity",
      Color{},
      "this is a point entity",
      {
        PropertyDefinition{"colorStr", PropertyValueTypes::String{}, "", ""},
        PropertyDefinition{"color1", PropertyValueTypes::Color<RgbF>{}, "", ""},
        PropertyDefinition{"color255", PropertyValueTypes::Color<RgbB>{}, "", ""},
        PropertyDefinition{"colorAny", PropertyValueTypes::Color<Rgb>{}, "", ""},
        PropertyDefinition{"color", PropertyValueTypes::Color<RgbF>{}, "", ""},
      },
      PointEntityDefinition{vm::bbox3d{64.0}, {}, {}},
    },
    {
      "color_entity2",
      Color{},
      "this is a point entity",
      {
        PropertyDefinition{"color", PropertyValueTypes::Color<RgbB>{}, "", ""},
      },
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
      CHECK(map.world()->defaultLayer()->children() == std::vector<Node*>{entityNode});
      CHECK(entityNode->entity().definition() == pointEntityDefinition);
      CHECK(entityNode->entity().origin() == vm::vec3d{16.0, 32.0, 48.0});
      CHECK(map.selection().nodes == std::vector<Node*>{entityNode});

      SECTION("Undo and redo")
      {
        map.undoCommand();
        CHECK(map.world()->defaultLayer()->children() == std::vector<Node*>{});
        CHECK(map.selection().nodes == std::vector<Node*>{});

        map.redoCommand();
        CHECK(map.world()->defaultLayer()->children() == std::vector<Node*>{entityNode});
        CHECK(entityNode->entity().definition() == pointEntityDefinition);
        CHECK(entityNode->entity().origin() == vm::vec3d{16.0, 32.0, 48.0});
        CHECK(map.selection().nodes == std::vector<Node*>{entityNode});
      }
    }

    SECTION("Selected objects are deselect and not translated")
    {
      auto* existingNode =
        createPointEntity(map, *pointEntityDefinition, vm::vec3d{0, 0, 0});
      selectNodes(map, {existingNode});

      const auto origin = existingNode->entity().origin();
      createPointEntity(map, *pointEntityDefinition, {16, 16, 16});

      CHECK(existingNode->entity().origin() == origin);
    }

    SECTION("Default entity properties")
    {
      auto fixtureConfig = MapFixtureConfig{};
      fixtureConfig.gameInfo.gameConfig.entityConfig.setDefaultProperties = true;

      auto& mapWithDefaultProperties = fixture.create(fixtureConfig);
      mapWithDefaultProperties.entityDefinitionManager().setDefinitions({
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
        mapWithDefaultProperties.entityDefinitionManager().definitions().front();

      auto* entityNode =
        createPointEntity(mapWithDefaultProperties, definitionWithDefaults, {0, 0, 0});
      REQUIRE(entityNode != nullptr);
      CHECK_THAT(
        entityNode->entity().properties(),
        UnorderedEquals(std::vector<EntityProperty>{
          {EntityPropertyKeys::Classname, "some_name"},
          {"some_default_prop", "value"},
        }));
    }

    SECTION("Linked group update failure")
    {
      auto* entityNode = new EntityNode{Entity{}};
      addNodes(map, {{parentForNodes(map), {entityNode}}});
      selectNodes(map, {entityNode});

      // move the entity down
      REQUIRE(translateSelection(map, {0, 0, -256}));
      REQUIRE(
        entityNode->physicalBounds() == vm::bbox3d{{-8, -8, -256 - 8}, {8, 8, -256 + 8}});

      auto* groupNode = groupSelectedNodes(map, "test");
      auto* linkedGroupNode = createLinkedDuplicate(map);
      REQUIRE(linkedGroupNode);

      // move the linked group up by half the world bounds
      const auto zOffset = map.worldBounds().max.z();
      deselectAll(map);
      selectNodes(map, {linkedGroupNode});
      translateSelection(map, {0, 0, map.worldBounds().max.z()});
      REQUIRE(
        linkedGroupNode->physicalBounds()
        == vm::bbox3d{{-8, -8, -256 - 8 + zOffset}, {8, 8, -256 + 8 + zOffset}});

      // create a brush entity inside the original group
      openGroup(map, *groupNode);
      deselectAll(map);

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

      selectNodes(map, {brushNode});
      auto* entityNode = createBrushEntity(map, *brushEntityDefinition);
      CHECK(entityNode != nullptr);
      CHECK(map.world()->defaultLayer()->children() == std::vector<Node*>{entityNode});
      CHECK(entityNode->children() == std::vector<Node*>{brushNode});
      CHECK(entityNode->entity().definition() == brushEntityDefinition);
      CHECK(map.selection().nodes == std::vector<Node*>{brushNode});

      SECTION("Undo and redo")
      {
        map.undoCommand();
        CHECK(map.world()->defaultLayer()->children() == std::vector<Node*>{brushNode});
        CHECK(map.selection().nodes == std::vector<Node*>{brushNode});

        map.redoCommand();
        CHECK(map.world()->defaultLayer()->children() == std::vector<Node*>{entityNode});
        CHECK(entityNode->children() == std::vector<Node*>{brushNode});
        CHECK(entityNode->entity().definition() == brushEntityDefinition);
        CHECK(map.selection().nodes == std::vector<Node*>{brushNode});
      }
    }

    SECTION("Copies properties from existing brush entity")
    {
      auto* brushNode1 = createBrushNode(map, "some_material");
      auto* brushNode2 = createBrushNode(map, "some_material");
      auto* brushNode3 = createBrushNode(map, "some_material");
      addNodes(map, {{parentForNodes(map), {brushNode1, brushNode2, brushNode3}}});

      selectNodes(map, {brushNode1, brushNode2, brushNode3});
      auto* previousEntityNode = createBrushEntity(map, *brushEntityDefinition);

      setEntityProperty(map, "prop", "value");
      REQUIRE(previousEntityNode->entity().hasProperty("prop", "value"));

      deselectAll(map);
      selectNodes(map, {brushNode1, brushNode2});

      auto* newEntityNode = createBrushEntity(map, *brushEntityDefinition);
      CHECK(newEntityNode != nullptr);
      CHECK(newEntityNode->entity().hasProperty("prop", "value"));
    }

    SECTION("Default entity properties")
    {
      auto fixtureConfig = MapFixtureConfig{};
      fixtureConfig.gameInfo.gameConfig.entityConfig.setDefaultProperties = true;

      auto& mapWithDefaultProperties = fixture.create(fixtureConfig);
      mapWithDefaultProperties.entityDefinitionManager().setDefinitions({
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
        mapWithDefaultProperties.entityDefinitionManager().definitions().front();

      auto* brushNode = createBrushNode(mapWithDefaultProperties, "some_material");
      addNodes(
        mapWithDefaultProperties,
        {{parentForNodes(mapWithDefaultProperties), {brushNode}}});

      selectNodes(mapWithDefaultProperties, {brushNode});
      auto* entityNode =
        createBrushEntity(mapWithDefaultProperties, definitionWithDefaults);
      REQUIRE(entityNode != nullptr);
      CHECK_THAT(
        entityNode->entity().properties(),
        UnorderedEquals(std::vector<EntityProperty>{
          {EntityPropertyKeys::Classname, "some_name"},
          {"some_default_prop", "value"},
        }));
    }

    SECTION("Linked group update failure")
    {
      auto* entityNode = new EntityNode{Entity{}};
      addNodes(map, {{parentForNodes(map), {entityNode}}});
      selectNodes(map, {entityNode});

      // move the entity down
      REQUIRE(translateSelection(map, {0, 0, -256}));
      REQUIRE(
        entityNode->physicalBounds() == vm::bbox3d{{-8, -8, -256 - 8}, {8, 8, -256 + 8}});

      auto* groupNode = groupSelectedNodes(map, "test");
      auto* linkedGroupNode = createLinkedDuplicate(map);

      // move the linked group up by half the world bounds
      const auto zOffset = map.worldBounds().max.z();
      deselectAll(map);
      selectNodes(map, {linkedGroupNode});
      translateSelection(map, {0, 0, map.worldBounds().max.z()});
      REQUIRE(
        linkedGroupNode->physicalBounds()
        == vm::bbox3d{{-8, -8, -256 - 8 + zOffset}, {8, 8, -256 + 8 + zOffset}});

      // create a brush entity inside the original group
      openGroup(map, *groupNode);
      deselectAll(map);

      auto* brushNode = createBrushNode(map);
      transformNode(
        *brushNode, vm::translation_matrix(vm::vec3d{0, 0, -32}), map.worldBounds());
      REQUIRE(brushNode->physicalBounds() == vm::bbox3d{{-16, -16, -48}, {16, 16, -16}});

      addNodes(map, {{parentForNodes(map), {brushNode}}});
      deselectAll(map);
      selectNodes(map, {brushNode});

      // create a brush entity - a temporarily empty entity will be created at the origin
      // and propagated into the linked group, where it ends up out of world bounds and
      // thus failing
      CHECK(createBrushEntity(map, *brushEntityDefinition) != nullptr);
    }
  }

  SECTION("setEntityProperty")
  {
    SECTION("Add an entity property")
    {
      const auto defaultToProtected = GENERATE(true, false);

      const auto originalEntity1 = Entity{};
      const auto originalEntity2 = Entity{{
        {"some_other_key", "some_other_value"},
      }};

      auto expectedEntity1 = Entity{{
        {"some_key", "some_value"},
      }};
      auto expectedEntity2 = Entity{{
        {"some_other_key", "some_other_value"},
        {"some_key", "some_value"},
      }};

      if (defaultToProtected)
      {
        expectedEntity1.setProtectedProperties({"some_key"});
        expectedEntity2.setProtectedProperties({"some_key"});
      }

      const auto entityNode1 = new EntityNode{originalEntity1};
      const auto entityNode2 = new EntityNode{originalEntity2};

      addNodes(map, {{parentForNodes(map), {entityNode1, entityNode2}}});

      selectNodes(map, {entityNode1, entityNode2});
      CHECK(setEntityProperty(map, "some_key", "some_value", defaultToProtected));
      CHECK(entityNode1->entity() == expectedEntity1);
      CHECK(entityNode2->entity() == expectedEntity2);

      SECTION("Undo and redo")
      {
        map.undoCommand();
        CHECK(map.selection().nodes == std::vector<Node*>{entityNode1, entityNode2});
        CHECK(entityNode1->entity() == originalEntity1);
        CHECK(entityNode2->entity() == originalEntity2);

        map.redoCommand();
        CHECK(map.selection().nodes == std::vector<Node*>{entityNode1, entityNode2});
        CHECK(entityNode1->entity() == expectedEntity1);
        CHECK(entityNode2->entity() == expectedEntity2);
      }
    }

    SECTION("Update an entity property")
    {
      const auto defaultToProtected = GENERATE(true, false);

      const auto originalEntity = Entity{{
        {"some_key", "some_other_value"},
      }};

      const auto expectedEntity = Entity{{
        {"some_key", "some_value"},
      }};

      const auto entityNode = new EntityNode{originalEntity};
      addNodes(map, {{parentForNodes(map), {entityNode}}});

      selectNodes(map, {entityNode});
      CHECK(setEntityProperty(map, "some_key", "some_value", defaultToProtected));
      CHECK(entityNode->entity() == expectedEntity);

      SECTION("Undo and redo")
      {
        map.undoCommand();
        CHECK(map.selection().nodes == std::vector<Node*>{entityNode});
        CHECK(entityNode->entity() == originalEntity);

        map.redoCommand();
        CHECK(map.selection().nodes == std::vector<Node*>{entityNode});
        CHECK(entityNode->entity() == expectedEntity);
      }
    }

    SECTION("Change entity class name")
    {
      auto* entityNode = new EntityNode(Entity{{{"classname", "large_entity"}}});

      addNodes(map, {{parentForNodes(map), {entityNode}}});
      REQUIRE(entityNode->entity().definition() == largeEntityDefinition);

      deselectAll(map);
      selectNodes(map, {entityNode});
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

      SECTION("Undo and redo")
      {
        map.undoCommand();
        CHECK(entityNode->entity().definition() == pointEntityDefinition);
        CHECK(
          map.selectionBounds()->size()
          == pointEntityDefinition->pointEntityDefinition->bounds.size());

        map.undoCommand();
        CHECK(entityNode->entity().definition() == largeEntityDefinition);
        CHECK(
          map.selectionBounds()->size()
          == largeEntityDefinition->pointEntityDefinition->bounds.size());

        map.redoCommand();
        CHECK(entityNode->entity().definition() == pointEntityDefinition);
        CHECK(
          map.selectionBounds()->size()
          == pointEntityDefinition->pointEntityDefinition->bounds.size());

        map.redoCommand();
        CHECK(entityNode->entity().definition() == nullptr);
        CHECK(map.selectionBounds()->size() == EntityNode::DefaultBounds.size());
      }
    }

    SECTION("Attempt to set a property with 2 out of 3 groups selected")
    {
      // https://github.com/TrenchBroom/TrenchBroom/issues/3768

      auto* entityNode = new EntityNode{Entity{}};
      addNodes(map, {{parentForNodes(map), {entityNode}}});
      selectNodes(map, {entityNode});

      auto* groupNode = groupSelectedNodes(map, "test");
      auto* linkedGroupNode1 = createLinkedDuplicate(map);
      auto* linkedGroupNode2 = createLinkedDuplicate(map);

      REQUIRE(groupNode != nullptr);
      REQUIRE(linkedGroupNode1 != nullptr);
      REQUIRE(linkedGroupNode2 != nullptr);

      deselectAll(map);

      selectNodes(map, {groupNode, linkedGroupNode1});

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
    SECTION("Rename entity property")
    {
      const auto originalEntity1 = Entity{{
        {"some_key", "some_value"},
      }};
      const auto originalEntity2 = Entity{{
        {"some_key", "some_value"},
        {"some_other_key", "some_other_value"},
      }};

      const auto expectedEntity1 = Entity{{
        {"some_other_key", "some_value"},
      }};
      const auto expectedEntity2 = Entity{{
        {"some_other_key", "some_value"},
      }};

      const auto entityNode1 = new EntityNode{originalEntity1};
      const auto entityNode2 = new EntityNode{originalEntity2};

      addNodes(map, {{parentForNodes(map), {entityNode1, entityNode2}}});

      selectNodes(map, {entityNode1, entityNode2});
      CHECK(renameEntityProperty(map, "some_key", "some_other_key"));
      CHECK(entityNode1->entity() == expectedEntity1);
      CHECK(entityNode2->entity() == expectedEntity2);

      SECTION("Undo and redo")
      {
        map.undoCommand();
        CHECK(map.selection().nodes == std::vector<Node*>{entityNode1, entityNode2});
        CHECK(entityNode1->entity() == originalEntity1);
        CHECK(entityNode2->entity() == originalEntity2);

        map.redoCommand();
        CHECK(map.selection().nodes == std::vector<Node*>{entityNode1, entityNode2});
        CHECK(entityNode1->entity() == expectedEntity1);
        CHECK(entityNode2->entity() == expectedEntity2);
      }
    }

    SECTION("Rename entity class name")
    {
      auto* entityNode = new EntityNode(Entity{{{"classname", "large_entity"}}});

      addNodes(map, {{parentForNodes(map), {entityNode}}});
      REQUIRE(entityNode->entity().definition() == largeEntityDefinition);

      deselectAll(map);
      selectNodes(map, {entityNode});
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

      SECTION("Undo and redo")
      {
        map.undoCommand();
        CHECK(map.selection().nodes == std::vector<Node*>{entityNode});
        CHECK(entityNode->entity().definition() == nullptr);
        CHECK(map.selectionBounds()->size() == EntityNode::DefaultBounds.size());

        map.undoCommand();
        CHECK(map.selection().nodes == std::vector<Node*>{entityNode});
        CHECK(entityNode->entity().definition() == largeEntityDefinition);
        CHECK(
          map.selectionBounds()->size()
          == largeEntityDefinition->pointEntityDefinition->bounds.size());

        map.redoCommand();
        CHECK(map.selection().nodes == std::vector<Node*>{entityNode});
        CHECK(entityNode->entity().definition() == nullptr);
        CHECK(map.selectionBounds()->size() == EntityNode::DefaultBounds.size());

        map.redoCommand();
        CHECK(map.selection().nodes == std::vector<Node*>{entityNode});
        CHECK(entityNode->entity().definition() == largeEntityDefinition);
        CHECK(
          map.selectionBounds()->size()
          == largeEntityDefinition->pointEntityDefinition->bounds.size());
      }
    }
  }

  SECTION("removeEntityProperty")
  {
    SECTION("Remove entity property")
    {
      const auto originalEntity1 = Entity{{
        {"some_key", "some_value"},
      }};
      const auto originalEntity2 = Entity{{
        {"some_key", "some_value"},
        {"some_other_key", "some_other_value"},
      }};

      const auto expectedEntity1 = Entity{{}};
      const auto expectedEntity2 = Entity{{
        {"some_other_key", "some_other_value"},
      }};

      const auto entityNode1 = new EntityNode{originalEntity1};
      const auto entityNode2 = new EntityNode{originalEntity2};

      addNodes(map, {{parentForNodes(map), {entityNode1, entityNode2}}});

      selectNodes(map, {entityNode1, entityNode2});
      CHECK(removeEntityProperty(map, "some_key"));
      CHECK(entityNode1->entity() == expectedEntity1);
      CHECK(entityNode2->entity() == expectedEntity2);

      SECTION("Undo and redo")
      {
        map.undoCommand();
        CHECK(map.selection().nodes == std::vector<Node*>{entityNode1, entityNode2});
        CHECK(entityNode1->entity() == originalEntity1);
        CHECK(entityNode2->entity() == originalEntity2);

        map.redoCommand();
        CHECK(map.selection().nodes == std::vector<Node*>{entityNode1, entityNode2});
        CHECK(entityNode1->entity() == expectedEntity1);
        CHECK(entityNode2->entity() == expectedEntity2);
      }
    }

    SECTION("Remove entity class name")
    {
      auto* entityNode = new EntityNode{Entity{{{"classname", "large_entity"}}}};

      addNodes(map, {{parentForNodes(map), {entityNode}}});
      REQUIRE(entityNode->entity().definition() == largeEntityDefinition);

      deselectAll(map);
      selectNodes(map, {entityNode});
      REQUIRE(
        map.selectionBounds()->size()
        == largeEntityDefinition->pointEntityDefinition->bounds.size());

      removeEntityProperty(map, "classname");
      CHECK(entityNode->entity().definition() == nullptr);
      CHECK(map.selectionBounds()->size() == EntityNode::DefaultBounds.size());

      SECTION("Undo and redo")
      {
        map.undoCommand();
        CHECK(map.selection().nodes == std::vector<Node*>{entityNode});
        CHECK(entityNode->entity().definition() == largeEntityDefinition);
        CHECK(
          map.selectionBounds()->size()
          == largeEntityDefinition->pointEntityDefinition->bounds.size());

        map.redoCommand();
        CHECK(map.selection().nodes == std::vector<Node*>{entityNode});
        CHECK(entityNode->entity().definition() == nullptr);
        CHECK(map.selectionBounds()->size() == EntityNode::DefaultBounds.size());
      }
    }
  }

  SECTION("setEntityColorProperty")
  {
    const auto originalEntity1 = Entity{{
      {"classname", "color_entity"},
      {"colorStr", "0 1 2 3 4"},
      {"color255", "0 1 2 3 4"},
      {"color1", "0.1 0.2 0.3 0.4"},
      {"colorAny", "0.1 0.2 0.3 0.4 0.5"},
    }};

    auto* entityNode = new EntityNode{originalEntity1};
    addNodes(map, {{parentForNodes(map), {entityNode}}});
    selectNodes(map, {entityNode});

    SECTION("single entity selected")
    {
      using T = std::tuple<std::string, Rgb, std::vector<EntityProperty>>;

      const auto [propertyKey, colorToSet, expectedProperties] = GENERATE(values<T>({
        {"colorStr",
         RgbB{5, 6, 7},
         {
           {"classname", "color_entity"},
           {"colorStr", "5 6 7 3 4"},
           {"color255", "0 1 2 3 4"},
           {"color1", "0.1 0.2 0.3 0.4"},
           {"colorAny", "0.1 0.2 0.3 0.4 0.5"},
         }},
        {"color255",
         RgbB{5, 6, 7},
         {
           {"classname", "color_entity"},
           {"colorStr", "0 1 2 3 4"},
           {"color255", "5 6 7 3 4"},
           {"color1", "0.1 0.2 0.3 0.4"},
           {"colorAny", "0.1 0.2 0.3 0.4 0.5"},
         }},
        {"color1",
         RgbF{0.5f, 0.6f, 0.7f},
         {
           {"classname", "color_entity"},
           {"colorStr", "0 1 2 3 4"},
           {"color255", "0 1 2 3 4"},
           {"color1", "0.5 0.6 0.7 0.4"},
           {"colorAny", "0.1 0.2 0.3 0.4 0.5"},
         }},
        {"colorAny",
         RgbF{0.5f, 0.6f, 0.7f},
         {
           {"classname", "color_entity"},
           {"colorStr", "0 1 2 3 4"},
           {"color255", "0 1 2 3 4"},
           {"color1", "0.1 0.2 0.3 0.4"},
           {"colorAny", "0.5 0.6 0.7 0.4 0.5"},
         }},
      }));

      CAPTURE(propertyKey, colorToSet);

      REQUIRE(setEntityColorProperty(map, propertyKey, colorToSet));
      CHECK(entityNode->entity().properties() == expectedProperties);
    }

    SECTION("multiple entities selected")
    {
      const auto originalEntity2 = Entity{{
        {"classname", "color_entity2"},
        {"color", "1 2 3 4"},
      }};

      auto* entityNode2 = new EntityNode{originalEntity2};
      addNodes(map, {{parentForNodes(map), {entityNode2}}});
      selectNodes(map, {entityNode2});

      REQUIRE(setEntityColorProperty(map, "color", RgbF{0.0f, 0.5f, 1.0f}));
      CHECK(
        entityNode->entity().properties()
        == std::vector<EntityProperty>{
          {"classname", "color_entity"},
          {"colorStr", "0 1 2 3 4"},
          {"color255", "0 1 2 3 4"},
          {"color1", "0.1 0.2 0.3 0.4"},
          {"colorAny", "0.1 0.2 0.3 0.4 0.5"},
          {"color", "0 0.5 1"},
        });
      CHECK(
        entityNode2->entity().properties()
        == std::vector<EntityProperty>{
          {"classname", "color_entity2"},
          {"color", "0 127 255 4"},
        });
    }
  }

  SECTION("convertEntityColorRange")
  {
    const auto originalEntity = Entity{{
      {"color_255", "0 127 255"},
      {"color_f", "0 0.49803922 1"},
    }};

    using T = std::tuple<std::string, ColorRange::Type, std::string>;
    const auto [key, range, expectedValue] = GENERATE(values<T>({
      {"color_255", ColorRange::Byte, "0 127 255"},
      {"color_255", ColorRange::Float, "0 0.49803922 1"},
      {"color_f", ColorRange::Float, "0 0.49803922 1"},
      {"color_f", ColorRange::Byte, "0 127 255"},
    }));

    CAPTURE(key, range);

    auto* entityNode = new EntityNode{originalEntity};
    addNodes(map, {{parentForNodes(map), {entityNode}}});
    selectNodes(map, {entityNode});

    REQUIRE(convertEntityColorRange(map, key, range));
    REQUIRE(entityNode->entity().property(key) != nullptr);
    CHECK(*entityNode->entity().property(key) == expectedValue);
  }

  SECTION("updateEntitySpawnflag")
  {
    SECTION("Brush entity")
    {
      auto* brushNode = new BrushNode{
        builder.createCuboid(vm::bbox3d{{0, 0, 0}, {64, 64, 64}}, "material")
        | kdl::value()};
      addNodes(map, {{parentForNodes(map), {brushNode}}});

      selectAllNodes(map);

      auto* brushEntNode = createBrushEntity(map, *brushEntityDefinition);
      REQUIRE(map.selection().nodes == std::vector<Node*>{brushNode});

      REQUIRE(!brushEntNode->entity().hasProperty("spawnflags"));
      CHECK(updateEntitySpawnflag(map, "spawnflags", 1, true));

      REQUIRE(brushEntNode->entity().hasProperty("spawnflags"));
      CHECK(*brushEntNode->entity().property("spawnflags") == "2");

      SECTION("Undo and redo")
      {
        map.undoCommand();
        CHECK(map.selection().nodes == std::vector<Node*>{brushNode});
        CHECK(!brushEntNode->entity().hasProperty("spawnflags"));

        map.redoCommand();
        CHECK(map.selection().nodes == std::vector<Node*>{brushNode});
        REQUIRE(brushEntNode->entity().hasProperty("spawnflags"));
        CHECK(*brushEntNode->entity().property("spawnflags") == "2");
      }
    }
  }

  SECTION("setProtectedEntityProperty")
  {
    SECTION("Toggle protected state")
    {
      auto* entityNode = new EntityNode{Entity{}};
      addNodes(map, {{parentForNodes(map), {entityNode}}});

      selectNodes(map, {entityNode});

      SECTION("Set protected property")
      {
        setProtectedEntityProperty(map, "some_key", true);
        CHECK_THAT(
          entityNode->entity().protectedProperties(),
          UnorderedEquals(std::vector<std::string>{"some_key"}));

        SECTION("Undo and redo")
        {
          map.undoCommand();
          CHECK_THAT(
            entityNode->entity().protectedProperties(),
            UnorderedEquals(std::vector<std::string>{}));

          map.redoCommand();
          CHECK_THAT(
            entityNode->entity().protectedProperties(),
            UnorderedEquals(std::vector<std::string>{"some_key"}));
        }
      }

      SECTION("Unset protected property")
      {
        setProtectedEntityProperty(map, "some_key", true);
        REQUIRE_THAT(
          entityNode->entity().protectedProperties(),
          UnorderedEquals(std::vector<std::string>{"some_key"}));

        // Ensure that the consecutive SwapNodeContentsCommands are not collated
        deselectAll(map);
        selectNodes(map, {entityNode});

        setProtectedEntityProperty(map, "some_key", false);
        CHECK_THAT(
          entityNode->entity().protectedProperties(),
          UnorderedEquals(std::vector<std::string>{}));

        SECTION("Undo and redo")
        {
          map.undoCommand();
          CHECK_THAT(
            entityNode->entity().protectedProperties(),
            UnorderedEquals(std::vector<std::string>{"some_key"}));

          map.redoCommand();
          CHECK_THAT(
            entityNode->entity().protectedProperties(),
            UnorderedEquals(std::vector<std::string>{}));
        }
      }
    }

    SECTION("Setting protected entity properties restores their values")
    {
      auto* entityNode = new EntityNode{Entity{{{"some_key", "some_value"}}}};
      addNodes(map, {{parentForNodes(map), {entityNode}}});

      selectNodes(map, {entityNode});
      auto* groupNode = groupSelectedNodes(map, "test");

      deselectAll(map);
      selectNodes(map, {groupNode});

      auto* linkedGroupNode = createLinkedDuplicate(map);
      REQUIRE(linkedGroupNode->childCount() == 1u);

      // both entities have the same value initially
      auto* linkedEntityNode =
        dynamic_cast<EntityNode*>(linkedGroupNode->children().front());
      REQUIRE(linkedEntityNode);
      REQUIRE_THAT(
        linkedEntityNode->entity().properties(),
        UnorderedEquals(std::vector<EntityProperty>{{"some_key", "some_value"}}));

      deselectAll(map);
      selectNodes(map, {linkedEntityNode});

      // set the property to protected in the linked entity and change its value
      setProtectedEntityProperty(map, "some_key", true);
      setEntityProperty(map, "some_key", "another_value");
      REQUIRE_THAT(
        linkedEntityNode->entity().properties(),
        UnorderedEquals(std::vector<EntityProperty>{{"some_key", "another_value"}}));

      // the value in the original entity remains unchanged
      entityNode = dynamic_cast<EntityNode*>(groupNode->children().front());
      REQUIRE_THAT(
        entityNode->entity().properties(),
        UnorderedEquals(std::vector<EntityProperty>{{"some_key", "some_value"}}));

      SECTION("When there is an unprotected property in the corresponding entity")
      {
        // set the property to unprotected, now the original value should be restored
        setProtectedEntityProperty(map, "some_key", false);

        entityNode = dynamic_cast<EntityNode*>(groupNode->children().front());
        CHECK_THAT(
          linkedEntityNode->entity().properties(),
          UnorderedEquals(std::vector<EntityProperty>{{"some_key", "some_value"}}));
        CHECK_THAT(
          entityNode->entity().properties(),
          UnorderedEquals(std::vector<EntityProperty>{{"some_key", "some_value"}}));
      }

      SECTION("When no corresponding entity with an unprotected property can be found")
      {
        // set the property to protected in the original entity too
        deselectAll(map);
        selectNodes(map, {entityNode});
        setProtectedEntityProperty(map, "some_key", true);

        linkedEntityNode = dynamic_cast<EntityNode*>(linkedGroupNode->children().front());
        REQUIRE_THAT(
          entityNode->entity().properties(),
          UnorderedEquals(std::vector<EntityProperty>{{"some_key", "some_value"}}));
        REQUIRE_THAT(
          linkedEntityNode->entity().properties(),
          UnorderedEquals(std::vector<EntityProperty>{{"some_key", "another_value"}}));

        deselectAll(map);
        selectNodes(map, {linkedEntityNode});
        setProtectedEntityProperty(map, "some_key", false);

        entityNode = dynamic_cast<EntityNode*>(groupNode->children().front());
        CHECK_THAT(
          entityNode->entity().properties(),
          UnorderedEquals(std::vector<EntityProperty>{{"some_key", "some_value"}}));
        CHECK_THAT(
          linkedEntityNode->entity().properties(),
          UnorderedEquals(std::vector<EntityProperty>{{"some_key", "another_value"}}));

        SECTION(
          "Setting the property to unprotected in the original entity will fetch the new "
          "value now")
        {
          deselectAll(map);
          selectNodes(map, {entityNode});
          setProtectedEntityProperty(map, "some_key", false);

          linkedEntityNode =
            dynamic_cast<EntityNode*>(linkedGroupNode->children().front());
          CHECK_THAT(
            entityNode->entity().properties(),
            UnorderedEquals(std::vector<EntityProperty>{{"some_key", "another_value"}}));
          CHECK_THAT(
            linkedEntityNode->entity().properties(),
            UnorderedEquals(std::vector<EntityProperty>{{"some_key", "another_value"}}));
        }
      }

      SECTION("When setting a property to unprotected that only exists in one entity")
      {
        setProtectedEntityProperty(map, "yet_another_key", true);
        setEntityProperty(map, "yet_another_key", "yet_another_value");

        entityNode = dynamic_cast<EntityNode*>(groupNode->children().front());
        REQUIRE_THAT(
          entityNode->entity().properties(),
          UnorderedEquals(std::vector<EntityProperty>{{"some_key", "some_value"}}));
        REQUIRE_THAT(
          linkedEntityNode->entity().properties(),
          UnorderedEquals(std::vector<EntityProperty>{
            {"some_key", "another_value"},
            {"yet_another_key", "yet_another_value"},
          }));

        setProtectedEntityProperty(map, "yet_another_key", false);

        entityNode = dynamic_cast<EntityNode*>(groupNode->children().front());
        CHECK_THAT(
          entityNode->entity().properties(),
          UnorderedEquals(std::vector<EntityProperty>{
            {"some_key", "some_value"},
            {"yet_another_key", "yet_another_value"},
          }));
        CHECK_THAT(
          linkedEntityNode->entity().properties(),
          UnorderedEquals(std::vector<EntityProperty>{
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

    selectNodes(map, {entityNode});
    CHECK(canClearProtectedEntityProperties(map));

    auto* groupNode = groupSelectedNodes(map, "test");

    deselectAll(map);
    selectNodes(map, {groupNode});
    CHECK(canClearProtectedEntityProperties(map));

    auto* linkedGroupNode = createLinkedDuplicate(map);
    REQUIRE(linkedGroupNode->childCount() == 1u);

    // both entities have the same values initially
    auto* linkedEntityNode =
      dynamic_cast<EntityNode*>(linkedGroupNode->children().front());
    REQUIRE(linkedEntityNode);

    deselectAll(map);
    selectNodes(map, {entityNode});

    // set the property "some_key" to protected in the original entity and change its
    // value
    setProtectedEntityProperty(map, "some_key", true);
    setEntityProperty(map, "some_key", "some_other_value");

    linkedEntityNode = dynamic_cast<EntityNode*>(linkedGroupNode->children().front());
    REQUIRE(linkedEntityNode);

    deselectAll(map);
    selectNodes(map, {linkedEntityNode});

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
      UnorderedEquals(std::vector<std::string>{"some_key"}));
    REQUIRE_THAT(
      entityNode->entity().properties(),
      UnorderedEquals(std::vector<EntityProperty>{
        {"some_key", "some_other_value"}, {"another_key", "another_value"}}));

    REQUIRE_THAT(
      linkedEntityNode->entity().protectedProperties(),
      UnorderedEquals(std::vector<std::string>{"another_key", "yet_another_key"}));
    REQUIRE_THAT(
      linkedEntityNode->entity().properties(),
      UnorderedEquals(std::vector<EntityProperty>{
        {"some_key", "some_value"},
        {"another_key", "yet_another_value"},
        {"yet_another_key", "and_yet_another_value"}}));

    deselectAll(map);
    selectNodes(map, {groupNode});
    selectNodes(map, {linkedGroupNode});

    CHECK_FALSE(canClearProtectedEntityProperties(map));

    deselectNodes(map, {groupNode});

    CHECK(canClearProtectedEntityProperties(map));
    clearProtectedEntityProperties(map);

    entityNode = dynamic_cast<EntityNode*>(groupNode->children().front());
    REQUIRE(entityNode != nullptr);

    CHECK_THAT(
      entityNode->entity().protectedProperties(),
      UnorderedEquals(std::vector<std::string>{"some_key"}));
    CHECK_THAT(
      entityNode->entity().properties(),
      UnorderedEquals(std::vector<EntityProperty>{
        {"some_key", "some_other_value"},
        {"another_key", "another_value"},
        {"yet_another_key", "and_yet_another_value"}}));

    CHECK_THAT(
      linkedEntityNode->entity().protectedProperties(),
      UnorderedEquals(std::vector<std::string>{}));
    CHECK_THAT(
      linkedEntityNode->entity().properties(),
      UnorderedEquals(std::vector<EntityProperty>{
        {"some_key", "some_value"},
        {"another_key", "another_value"},
        {"yet_another_key", "and_yet_another_value"}}));

    SECTION("Undo and redo")
    {
      map.undoCommand();

      entityNode = dynamic_cast<EntityNode*>(groupNode->children().front());
      REQUIRE(entityNode != nullptr);

      CHECK_THAT(
        entityNode->entity().protectedProperties(),
        UnorderedEquals(std::vector<std::string>{"some_key"}));
      CHECK_THAT(
        entityNode->entity().properties(),
        UnorderedEquals(std::vector<EntityProperty>{
          {"some_key", "some_other_value"}, {"another_key", "another_value"}}));

      CHECK_THAT(
        linkedEntityNode->entity().protectedProperties(),
        UnorderedEquals(std::vector<std::string>{"another_key", "yet_another_key"}));
      CHECK_THAT(
        linkedEntityNode->entity().properties(),
        UnorderedEquals(std::vector<EntityProperty>{
          {"some_key", "some_value"},
          {"another_key", "yet_another_value"},
          {"yet_another_key", "and_yet_another_value"}}));

      map.redoCommand();
      entityNode = dynamic_cast<EntityNode*>(groupNode->children().front());
      REQUIRE(entityNode != nullptr);

      CHECK_THAT(
        entityNode->entity().protectedProperties(),
        UnorderedEquals(std::vector<std::string>{"some_key"}));
      CHECK_THAT(
        entityNode->entity().properties(),
        UnorderedEquals(std::vector<EntityProperty>{
          {"some_key", "some_other_value"},
          {"another_key", "another_value"},
          {"yet_another_key", "and_yet_another_value"}}));

      CHECK_THAT(
        linkedEntityNode->entity().protectedProperties(),
        UnorderedEquals(std::vector<std::string>{}));
      CHECK_THAT(
        linkedEntityNode->entity().properties(),
        UnorderedEquals(std::vector<EntityProperty>{
          {"some_key", "some_value"},
          {"another_key", "another_value"},
          {"yet_another_key", "and_yet_another_value"}}));
    }
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
    selectNodes(map, {entityNodeWithoutDefinition});
    setEntityProperty(map, "some_prop", "some_value");
    deselectAll(map);

    auto* entityNodeWithProp = createPointEntity(map, *definitionWithDefaults, {0, 0, 0});
    REQUIRE(entityNodeWithProp != nullptr);
    REQUIRE(entityNodeWithProp->entity().definition() == definitionWithDefaults);
    selectNodes(map, {entityNodeWithProp});
    setEntityProperty(map, "some_prop", "some_value");
    deselectAll(map);

    auto* entityNodeWithPropA =
      createPointEntity(map, *definitionWithDefaults, {0, 0, 0});
    REQUIRE(entityNodeWithPropA != nullptr);
    REQUIRE(entityNodeWithPropA->entity().definition() == definitionWithDefaults);
    selectNodes(map, {entityNodeWithPropA});
    setEntityProperty(map, "some_prop", "some_value");
    setEntityProperty(map, "default_prop_a", "default_value_a");
    deselectAll(map);

    auto* entityNodeWithPropAWithValueChanged =
      createPointEntity(map, *definitionWithDefaults, {0, 0, 0});
    REQUIRE(entityNodeWithPropAWithValueChanged != nullptr);
    REQUIRE(
      entityNodeWithPropAWithValueChanged->entity().definition()
      == definitionWithDefaults);
    selectNodes(map, {entityNodeWithPropAWithValueChanged});
    setEntityProperty(map, "default_prop_a", "some_other_value");
    deselectAll(map);

    auto* entityNodeWithPropsAB =
      createPointEntity(map, *definitionWithDefaults, {0, 0, 0});
    REQUIRE(entityNodeWithPropsAB != nullptr);
    REQUIRE(entityNodeWithPropsAB->entity().definition() == definitionWithDefaults);
    selectNodes(map, {entityNodeWithPropsAB});
    setEntityProperty(map, "some_prop", "some_value");
    setEntityProperty(map, "default_prop_a", "default_value_a");
    setEntityProperty(map, "default_prop_b", "yet_another_value");
    deselectAll(map);

    REQUIRE_THAT(
      entityNodeWithoutDefinition->entity().properties(),
      UnorderedEquals(std::vector<EntityProperty>{
        {"classname", "some_class"},
        {"some_prop", "some_value"},
      }));
    REQUIRE_THAT(
      entityNodeWithProp->entity().properties(),
      UnorderedEquals(std::vector<EntityProperty>{
        {"classname", "some_name"},
        {"some_prop", "some_value"},
      }));
    REQUIRE_THAT(
      entityNodeWithPropA->entity().properties(),
      UnorderedEquals(std::vector<EntityProperty>{
        {"classname", "some_name"},
        {"some_prop", "some_value"},
        {"default_prop_a", "default_value_a"},
      }));
    REQUIRE_THAT(
      entityNodeWithPropAWithValueChanged->entity().properties(),
      UnorderedEquals(std::vector<EntityProperty>{
        {"classname", "some_name"},
        {"default_prop_a", "some_other_value"},
      }));
    REQUIRE_THAT(
      entityNodeWithPropsAB->entity().properties(),
      UnorderedEquals(std::vector<EntityProperty>{
        {"classname", "some_name"},
        {"some_prop", "some_value"},
        {"default_prop_a", "default_value_a"},
        {"default_prop_b", "yet_another_value"},
      }));

    selectNodes(
      map,
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
        UnorderedEquals(std::vector<EntityProperty>{
          {"classname", "some_class"},
          {"some_prop", "some_value"},
        }));
      CHECK_THAT(
        entityNodeWithProp->entity().properties(),
        UnorderedEquals(std::vector<EntityProperty>{
          {"classname", "some_name"},
          {"some_prop", "some_value"},
        }));
      CHECK_THAT(
        entityNodeWithPropA->entity().properties(),
        UnorderedEquals(std::vector<EntityProperty>{
          {"classname", "some_name"},
          {"some_prop", "some_value"},
          {"default_prop_a", "default_value_a"},
        }));
      CHECK_THAT(
        entityNodeWithPropAWithValueChanged->entity().properties(),
        UnorderedEquals(std::vector<EntityProperty>{
          {"classname", "some_name"},
          {"default_prop_a", "default_value_a"},
        }));
      CHECK_THAT(
        entityNodeWithPropsAB->entity().properties(),
        UnorderedEquals(std::vector<EntityProperty>{
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
        UnorderedEquals(std::vector<EntityProperty>{
          {"classname", "some_class"},
          {"some_prop", "some_value"},
        }));
      CHECK_THAT(
        entityNodeWithProp->entity().properties(),
        UnorderedEquals(std::vector<EntityProperty>{
          {"classname", "some_name"},
          {"some_prop", "some_value"},
          {"default_prop_a", "default_value_a"},
          {"default_prop_b", "default_value_b"},
        }));
      CHECK_THAT(
        entityNodeWithPropA->entity().properties(),
        UnorderedEquals(std::vector<EntityProperty>{
          {"classname", "some_name"},
          {"some_prop", "some_value"},
          {"default_prop_a", "default_value_a"},
          {"default_prop_b", "default_value_b"},
        }));
      CHECK_THAT(
        entityNodeWithPropAWithValueChanged->entity().properties(),
        UnorderedEquals(std::vector<EntityProperty>{
          {"classname", "some_name"},
          {"default_prop_a", "some_other_value"},
          {"default_prop_b", "default_value_b"},
        }));
      CHECK_THAT(
        entityNodeWithPropsAB->entity().properties(),
        UnorderedEquals(std::vector<EntityProperty>{
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
        UnorderedEquals(std::vector<EntityProperty>{
          {"classname", "some_class"},
          {"some_prop", "some_value"},
        }));
      CHECK_THAT(
        entityNodeWithProp->entity().properties(),
        UnorderedEquals(std::vector<EntityProperty>{
          {"classname", "some_name"},
          {"some_prop", "some_value"},
          {"default_prop_a", "default_value_a"},
          {"default_prop_b", "default_value_b"},
        }));
      CHECK_THAT(
        entityNodeWithPropA->entity().properties(),
        UnorderedEquals(std::vector<EntityProperty>{
          {"classname", "some_name"},
          {"some_prop", "some_value"},
          {"default_prop_a", "default_value_a"},
          {"default_prop_b", "default_value_b"},
        }));
      CHECK_THAT(
        entityNodeWithPropAWithValueChanged->entity().properties(),
        UnorderedEquals(std::vector<EntityProperty>{
          {"classname", "some_name"},
          {"default_prop_a", "default_value_a"},
          {"default_prop_b", "default_value_b"},
        }));
      CHECK_THAT(
        entityNodeWithPropsAB->entity().properties(),
        UnorderedEquals(std::vector<EntityProperty>{
          {"classname", "some_name"},
          {"some_prop", "some_value"},
          {"default_prop_a", "default_value_a"},
          {"default_prop_b", "default_value_b"},
        }));
    }
  }
}

} // namespace tb::mdl
